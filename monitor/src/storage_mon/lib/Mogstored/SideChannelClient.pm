### simple package for handling the stream request port
package Mogstored::SideChannelClient;

use strict;
use base qw{Perlbal::Socket};
use fields (
            'count',      # how many requests we've serviced
            'read_buf',   # unprocessed read buffer
            'mogsvc',     # the mogstored Perlbal::Service object
            );

# needed since we're pretending to be a Perlbal::Socket... never idle out
sub max_idle_time { return 0; }

sub new {
    my Mogstored::SideChannelClient $self = shift;
    $self = fields::new($self) unless ref $self;
    $self->SUPER::new(@_);
    $self->{count} = 0;
    $self->{read_buf} = '';
    $self->{mogsvc} = Perlbal->service('mogstored');
    return $self;
}

sub event_read {
    my Mogstored::SideChannelClient $self = shift;

    my $bref = $self->read(1024);
    return $self->close unless defined $bref;
    $self->{read_buf} .= $$bref;

    my $path = $self->{mogsvc}->{docroot};

    while ($self->{read_buf} =~ s/^(.+?)\r?\n//) {
        my $cmd = $1;
        if ($cmd =~ /^size (\S+)$/) {
            # increase our count
            $self->{count}++;

            # validate uri
            my $uri = $1;
            if ($uri =~ /\.\./) {
                $self->write("ERROR: uri invalid (contains ..)\r\n");
                return;
            }

            # now stat the file to get the size and such
            Perlbal::AIO::aio_stat("$path$uri", sub {
                return if $self->{closed};
                my $size = -e _ ? -s _ : -1;
                $self->write("$uri $size\r\n");
            });
        } elsif ($cmd =~ /^watch$/i) {
            unless (Mogstored->iostat_available) {
                $self->write("ERR iostat unavailable\r\n");
                next;
            }
            $self->watch_read(0);
            Mogstored->iostat_subscribe($self);
        } elsif (my ($start, $end, $raw_devs) = 
                 $cmd =~ /^fid_sizes \s+ (\d+)-(\d+) \s+ ([\d\s]+)$/ix) {
            my @devs = split /\s+/, $raw_devs;
            my $args = Storable::nfreeze([$start, $end, \@devs]);
            my $task = Gearman::Task->new(
                "fid_sizes" => \$args,
                {
                    on_complete => sub {
                        my $res = shift;
                        my $devices = Storable::thaw($$res);
                        foreach my $device_ent (@$devices) {
                            my ($device, $entries) = @$device_ent;
                            foreach my $entry (@$entries) {
                                my ($fid, $size) = @$entry;
                                $self->write("$device\t$fid\t$size\n");
                            }
                        }
                        $self->write(".\n");
                        $self->watch_read(1);
                    },
                    on_fail => sub {
                        $self->write("ERR error processing fid_sizes request\n");
                        $self->watch_read(1);
                    },
                });
            Mogstored->gearman_client->add_task($task);
            $self->watch_read(0);
		} elsif($cmd=~/^kill/i) {
			# 处理 runprocs 发出的 kill 命令
			$self->close;
			exit(0);
		} elsif($cmd=~/^exit\s+(\d+)/i) {
			# 处理 runprocs 发出的 exit 命令
			$self->close;
			exit($1);
		} elsif($cmd=~/^ruok/i) {
			# 处理 runprocs 发出的 ruok 命令
			$self->write("imok\n");
        } else {
            # we don't understand this so pass it on to manage command interface
            my @out;
            Perlbal::run_manage_command($cmd, sub { push @out, $_[0]; });
            $self->write(join("\r\n", @out) . "\r\n");
        }
    }
}

# stop watching writability if we've nothing else to
# write to them.  else just kick off more writes.
sub event_write {
    my $self = shift;
    $self->watch_write(0) if $self->write(undef);
}

# override Danga::Socket's event handlers which die
sub event_err { $_[0]->close; }
sub event_hup { $_[0]->close; }

# as_string handler
sub as_string {
    my Mogstored::SideChannelClient $self = shift;

    my $ret = $self->SUPER::as_string;
    $ret .= "; size_requests=$self->{count}";

    return $ret;
}

sub close {
    my Mogstored::SideChannelClient $self = shift;
    Mogstored->iostat_unsubscribe($self);
    $self->SUPER::close;
}

sub die_gracefully {
    Mogstored->on_sidechannel_die_gracefully;
}

1;
