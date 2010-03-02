package MogileFS::Worker::Monitor;
use strict;
use warnings;

use base 'MogileFS::Worker';
use fields (
            'last_db_update',  # devid -> time.  update db less often than poll interval.
            'last_test_write', # devid -> time.  time we last tried writing to a device.
            'skip_host',       # hostid -> 1 if already noted dead (reset every loop)
            'seen_hosts',      # IP -> 1 (reset every loop)
            'ua',              # LWP::UserAgent for checking usage files
            'iow',             # MogileFS::IOStatWatcher object
            );

use Danga::Socket 1.56;
use MogileFS::Util qw(error debug);
use MogileFS::IOStatWatcher;
# XXX: 用配置模块读取当前Tracker对应的卷ID
use MogileFS::Config;

use constant UPDATE_DB_EVERY => 15; 	# 更新元数据库中设备和主机状态信息的时间间隔
use constant MONITOR_INTERVAL => 2.5; 	# 最小状态检测时间间隔
use constant DEVICE_GET_URL_TIMEOUT => 60; 	# 尝试获取特定存储设备上磁盘使用率文件内容的超时时间

sub new {
    my ($class, $psock) = @_;
    my $self = fields::new($class);
    $self->SUPER::new($psock);

    $self->{last_db_update}  = {};
    $self->{last_test_write} = {};
    $self->{iow}             = MogileFS::IOStatWatcher->new;
    return $self;
}

sub watchdog_timeout {
    30;
}

sub work {
    my $self = shift;

    # we just forked from our parent process, also using Danga::Socket,
    # so we need to lose all that state and start afresh.
    Danga::Socket->Reset;

    my $iow = $self->{iow};
    $iow->on_stats(sub {
        my ($hostname, $stats) = @_;

        while (my ($devid, $util) = each %$stats) {
            my $dev = MogileFS::Device->of_devid($devid) or die "Can't find that device";
            $dev->set_observed_utilization($util);
        }
    });

    my $main_monitor;
    $main_monitor = sub {
        Danga::Socket->AddTimer(MONITOR_INTERVAL, $main_monitor);
        $self->parent_ping;

        # get db and note we're starting a run
        debug("Monitor running; scanning usage files");
        $self->validate_dbh;

        $self->{skip_host}  = {};  # hostid -> 1 if already noted dead.
        $self->{seen_hosts} = {}; # IP -> 1

        # now iterate over devices
        MogileFS::Device->invalidate_cache;
        MogileFS::Host->invalidate_cache;

        foreach my $dev (MogileFS::Device->devices) {
            next unless $dev->dstate->should_monitor;
            next if $self->{skip_host}{$dev->hostid};
            $self->check_device($dev);
        }

        $iow->set_hosts(keys %{$self->{seen_hosts}});
        $self->send_to_parent(":monitor_just_ran");
    };

    $main_monitor->();
    Danga::Socket->EventLoop;
}

# --------------------------------------------------------------------------

sub ua {
    my $self = shift;
    return $self->{ua} ||= LWP::UserAgent->new(
                                               timeout    => DEVICE_GET_URL_TIMEOUT,
                                               keep_alive => 20,
                                               );
}

sub check_device {
    my ($self, $dev) = @_;

    my $devid = $dev->id;
    my $host  = $dev->host;

    my $port     = $host->http_port;
    my $get_port = $host->http_get_port;
    my $hostip   = $host->ip;
    my $url      = $dev->usage_url;

    $self->{seen_hosts}{$hostip} = 1;

    # now try to get the data with a short timeout
    my $timeout = DEVICE_GET_URL_TIMEOUT;
    my $start_time = Time::HiRes::time();

    my $ua       = $self->ua;
    my $response = $ua->get($url);
    my $res_time = Time::HiRes::time();

    unless ($response->is_success) {
        my $failed_after = $res_time - $start_time;
        if ($failed_after < 0.5) {
            $self->broadcast_device_unreachable($dev->id);
            error("Port $get_port not listening on otherwise-alive machine $hostip?  Error was: " . $response->status_line);
        } else {
            $failed_after = sprintf("%.02f", $failed_after);
            $self->broadcast_host_unreachable($dev->hostid);
            $self->{skip_host}{$dev->hostid} = 1;
            error("Timeout contacting machine $hostip for dev $devid:  took $failed_after seconds out of $timeout allowed");
        }
        return;
    }

    # at this point we can reach the host
    $self->broadcast_host_reachable($dev->hostid);
    $self->{iow}->restart_monitoring_if_needed($hostip);

    my %stats;
    my $data = $response->content;
    foreach (split(/\r?\n/, $data)) {
        next unless /^(\w+)\s*:\s*(.+)$/;
        $stats{$1} = $2;
    }

    my ($used, $total) = ($stats{used}, $stats{total});
    unless ($used && $total) {
        $used  = "<undef>" unless defined $used;
        $total = "<undef>" unless defined $total;
        my $clen = length($data || "");
        error("dev$devid reports used = $used, total = $total, content-length: $clen, error?");
        return;
    }

    # only update database every ~15 seconds per device
    my $last_update = $self->{last_db_update}{$dev->id} || 0;
    my $next_update = $last_update + UPDATE_DB_EVERY;
    my $now = time();
    if ($now >= $next_update) {
        Mgd::get_store()->update_device_usage(mb_total => int($total / 1024),
                                              mb_used  => int($used / 1024),
                                              devid    => $devid);
        $self->{last_db_update}{$devid} = $now;
    }

    # next if we're not going to try this now
    return if ($self->{last_test_write}{$devid} || 0) + UPDATE_DB_EVERY > $now;
    $self->{last_test_write}{$devid} = $now;

    # now we want to check if this device is writeable

    # first, create the test-write directory.  this will return
    # immediately after the first time, as the 'create_directory'
    # function caches what it's already created.
	# XXX: 测试设备可写的临时文件目录要被卷目录隔离开，以免不同卷的写入测试产生竞争
	my $volid=MogileFS->config("volume_id");
    $dev->create_directory("/dev$devid/vol$volid/test-write");

    my $num = int(rand 10000);  # this was "$$-$now" before, but we don't yet have a cleaner in mogstored for these files
    my $puturl = "http://$hostip:$port/dev$devid/vol$volid/test-write/test-write-$num";
    my $content = "time=$now rand=$num";
    my $req = HTTP::Request->new(PUT => $puturl);
    $req->content($content);

    # TODO: guard against race-conditions with double-check on failure

    # now, depending on what happens
    my $resp = $ua->request($req);
    if ($resp->is_success) {
        # now let's get it back to verify; note we use the get_port to verify that
        # the distinction works (if we have one)
        my $geturl = "http://$hostip:$get_port/dev$devid/vol$volid/test-write/test-write-$num";
        my $testwrite = $ua->get($geturl);

        # if success and the content matches, mark it writeable
        if ($testwrite->is_success && $testwrite->content eq $content) {
            $self->broadcast_device_writeable($devid);
            debug("dev$devid: used = $used, total = $total, writeable = 1");
            return;
        }
    }

    # if we fall through to here, then we know that something is not so good, so mark it readable
    # which is guaranteed given we even tested writeability
    $self->broadcast_device_readable($devid);
    debug("dev$devid: used = $used, total = $total, writeable = 0");
}

1;

# Local Variables:
# mode: perl
# c-basic-indent: 4
# indent-tabs-mode: nil
# End:
