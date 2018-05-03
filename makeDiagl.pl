#!/usr/bin/perl

use strict;
use warnings;

my $i;
my $j;

for ($j = 14; $j <= 27; $j++) {
    for ($i = $j; $i >= $j-13; $i--) {
        printf ("%2s,", $i);
    }
    print "\n";
}

for ($j = 1; $j <= 14; $j++) {
    for ($i = $j; $i <= $j+13; $i++) {
        printf ("%2s,", $i);
    }
    print "\n";
}

