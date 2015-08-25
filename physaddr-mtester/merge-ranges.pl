#!/usr/bin/perl -lw

my %ranges;
my $min_block = 128 * 1024;

for (<>) {
    next if /^;;/;
    chomp;
    my ($start, $end) = map { hex } split /:/;
    # Round start down to the next $min_block boundary.
    $start &= ~($min_block - 1);
    # And end up to the next boundary.
    $end = ($end + $min_block - 1) & ~($min_block - 1);

    $ranges{sprintf "0x%08x:0x%08x", $start, $end} = 1;
}

my ($merge_start, $merge_end);

sub finish {
    return if !defined $merge_start;
    
    print "$merge_start:$merge_end";
    $merge_start = $merge_end = undef;
}

for (sort keys %ranges) {
    my ($start, $end) = split /:/;
    if (defined $merge_start and hex($start) == hex($merge_end) + 1) {
        finish;
    } else {
        finish;
        $merge_start = $start;
        $merge_end = $end;
    }
}

finish;
