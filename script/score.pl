#!/usr/bin/perl -w

####################################
# score.pl usage
####################################

$HELP = 0;
if (@ARGV != 2) {
  $HELP = 1;
}

$DEBUG = 0;
$i= 0;
while ($i <= $#ARGV) {
  if ($ARGV[$i] eq "-h") {
    $HELP = 1;
  }
  $i++;
}

if ($HELP) {
   print "   Useage: score.pl case.in case.out\n";
   print "   DESCRIPTION\n";
   print "       [case.in] \n";
   print "            Specify case input file\n";
   print "       [case.out] \n";
   print "            Specify case output file\n";
   exit 0;
}

####################################
print "Specified params: @ARGV\n";

open (INF, "<$ARGV[0]") or die "Error: Cannot open file $ARGV[0], $!";
open (OUTF, "<$ARGV[1]") or die "Error: Cannot open file $ARGV[1], $!";

$firstline = <INF>;
( $M, $N, $NCells ) = split(' ', $firstline);
print "Num_Rows = $M Num_Cols = $N Num_Cells = $NCells\n";
$M *= 8;

my @map;
for ( $col = 0; $col < $N; $col ++ ) {
    for ( $row = 0; $row < $M; $row ++ ) {
        $map[$col][$row] = 0;
    } 
}

$score = 0;
for ( $i = 0; $i < $NCells; $i ++ ) {
    if( !( $inline = <INF> ) ) {
        die "Error reading file $ARGV[0] for cell $i";
    }
    if( !( $outline = <OUTF> ) ) {
        die "Error reading file $ARGV[1] for cell $i";
    }

    ( $w, $x1, $y1 ) = split(' ', $inline);
    ( $x2, $y2 ) = split(' ', $outline);

    $score += $w * (( $x1 - $x2 ) * ( $x1 - $x2 ) + ( $y1 - $y2 ) * ( $y1 - $y2 ));

    if( $DEBUG == 1) {
        print "cell $i: width = $w; x1 = $x1; y1 = $y1; x2 = $x2; y2 = $y2; score = $score\n";
    }

    $col = $x2 / 8;
    if( $x2 != $col * 8) {
        die "Error: Cell $i has invalid x coordinate $x2.\n";
    }

    if( $x2 < 0 || $x2 + 8 > $N * 8 || $y2 < 0 || $y2 + $w > $M) {
        die "Error: Cell $i 's bound box outside chip area.\n";
    }

    for( $row = $y2; $row < $y2 + $w; $row ++ ) {
        if ( $map[$col][$row] != 0) {
            $cid = $map[$col][$row] - 1;
            die "Error: Found overlap between cell $cid and $i.\n"
        }
        else {
            $map[$col][$row] = $i + 1;
        }
    }
}

print "Score = $score\n";

close INF;
close OUTF;

exit $score;
