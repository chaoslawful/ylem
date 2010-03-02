# 定期查找元数据库中位于已失效存储设备上的文件副本列表，将其加入待冗余队列并同
# 失效存储设备解除关联。
# 查找间隔时间固定为5s，每次查找都遍历所有被标记为失效的存储设备，但在每个设备上
# 最多处理1000个文件，以免文件数过多导致无暇更新进程看门狗而被主进程杀死。

package MogileFS::Worker::Reaper;
# deletes files

use strict;
use base 'MogileFS::Worker';
use MogileFS::Util qw(every error debug);
use MogileFS::Config qw(DEVICE_SUMMARY_CACHE_TIMEOUT);

use constant REAP_INTERVAL => 5; 	# 清理失效设备上文件的间隔时间

sub new {
    my ($class, $psock) = @_;
    my $self = fields::new($class);
    $self->SUPER::new($psock);

    return $self;
}

my %all_empty;  # devid -> bool, if all empty of files in file_on

sub work {
    my $self = shift;

	# 每隔5s进行一次查找过程
    every(REAP_INTERVAL, sub {
		# 查找开始前先喂狗，表明进程仍然活动
        $self->parent_ping;

        # get db and note we're starting a run
        debug("Reaper running; looking for dead devices");

		# 遍历元数据库中每个被标记为永久失效的存储设备
        foreach my $dev (grep { $_->dstate->is_perm_dead }
                         MogileFS::Device->devices)
        {
            my $devid = $dev->id;
            next if $all_empty{$devid};

			# 取得永久失效设备上之前有记录的文件列表，最多1000个
            my @fids = $dev->fid_list(limit => 1000);
            unless (@fids) {
                $all_empty{$devid} = 1;
                next;
            }

            foreach my $fid (@fids) {
                # order is important here:

                # first, add fid to file_to_replicate table.  it
                # shouldn't matter if the replicator gets to this
                # before the subsequent 'forget_about' method, as the
                # replicator will treat dead file_on devices as
                # non-existent anyway.  however, it is important that
                # we enqueue it for replication first, before we
                # forget about that file_on row, otherwise a failure
                # after/during 'forget_about' could leave a stranded
                # file on a dead device and we'd never fix it.
                #
                # and just for extra safety, in case replication happened
                # on another machine after 'enqueue_for_replication' but
                # before 'forget_about', and that other machine hadn't yet
                # re-read the device table to learn that this device
                # was dead, we delay the replication for the amount of time
                # that the device summary table is valid for (presumably
                # the other trackers are running identical software, or
                # at least have the same timeout value)

                $fid->enqueue_for_replication(in => DEVICE_SUMMARY_CACHE_TIMEOUT + 1);
                $dev->forget_about($fid);
                $fid->update_devcount;
            }
        }
    });
}

1;

# Local Variables:
# mode: perl
# c-basic-indent: 4
# indent-tabs-mode: nil
# End:
