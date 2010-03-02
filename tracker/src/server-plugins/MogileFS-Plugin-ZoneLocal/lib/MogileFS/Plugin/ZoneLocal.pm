# ZoneLocal plugin for MogileFS, by hachi

package MogileFS::Plugin::ZoneLocal;

use strict;
use warnings;

our $VERSION = '0.01';
$VERSION = eval $VERSION;

use MogileFS::Worker::Query;
use MogileFS::Network;

sub prioritize_devs_current_zone;

sub load {
    MogileFS::register_global_hook( 'cmd_get_paths_order_devices', sub {
        my $devices = shift;
        my $sorted_devs = shift;

        @$sorted_devs = prioritize_devs_current_zone
                        MogileFS::Worker::Query::sort_devs_by_utilization(@$devices);

        return 1;
    });

    MogileFS::register_global_hook( 'cmd_create_open_order_devices', sub {
        my $devices = shift;
        my $sorted_devs = shift;

        @$sorted_devs = prioritize_devs_current_zone
                        MogileFS::Worker::Query::sort_devs_by_freespace(@$devices);

        return 1;
    });

    return 1;
}

sub unload {
    # remove our hooks
    MogileFS::unregister_global_hook( 'cmd_get_paths_order_devices' );

    return 1;
}

sub prioritize_devs_current_zone {
    my $current_zone = MogileFS::Network->zone_for_ip($MogileFS::REQ_client_ip);
    my (@this_zone, @other_zone);

    foreach my $dev (@_) {
        my $ip = $dev->host->ip;
        my $host_id = $dev->host->id;
        my $zone = MogileFS::Network->zone_for_ip($ip);

        if ($current_zone eq $zone) {
            push @this_zone, $dev;
        } else {
            push @other_zone, $dev;
        }
    }

    return @this_zone, @other_zone;
}

1;
