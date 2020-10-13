#!/usr/bin/perl -w

####################################
# runtest.pl usage
####################################

$HELP = 0;
if (@ARGV != 2) {
  $HELP = 1;
}

#$DEBUG = 1;
$i = 0;
while ($i <= $#ARGV) {
  if ($ARGV[$i] eq "-h") {
    $HELP = 1;
  }
  $i++;
}

if ($HELP) {
   print "   Useage: runtest.pl exe case.list\n";
   print "   DESCRIPTION\n";
   print "       [exe] \n";
   print "            Specify exe used for testing\n";
   print "       [case.list] \n";
   print "            Specify one file containing a list of cases for testing\n";
   exit 0;
}

##### ###############################
print "Specified params: @ARGV\n";

my ($sec,$min,$hour,$day,$mon,$year,$weekday,$yeardate,$savinglightday) = (localtime(time));
$sec    =    ($sec    <    10)?    "0$sec":$sec;
$min    =    ($min    <    10)?    "0$min":$min;
$hour   =    ($hour   <    10)?    "0$hour":$hour;
$day    =    ($day    <    10)?    "0$day":$day;
$mon    =    ($mon    <    9)?     "0".($mon+1):($mon+1);
$year   +=    1900;
$date   = "$year$mon$day\_$hour$min$sec";

$pwd = $ENV{'PWD'};
$dir = "$pwd/runtest_$date";
mkdir( $dir ) or die "Error: Cannot create run directory $dir.";
print "Created run directory $dir.\n";

$summary = "$dir/summary";
open (SUMMARY, ">$summary") or die "Error: Cannot create file $summary.";

$exe = $ARGV[0];
if( substr( $exe, 0, 1) ne "/" ) {
    $exe = "$pwd/$exe";
}

if( !( -x $exe)) {
    die "Error: Cannot find executable $exe.";
}
print "Use executable $exe for testing.\n";
print SUMMARY "Executable: $exe\n";

$caselist = $ARGV[1];
if( substr( $caselist, 0, 1) ne "/" ) {
    $caselist = "$pwd/$caselist";
}

open (CASELIST, "<$caselist") or die "Error: Cannot open case list $caselist.";
print SUMMARY "Case list: $caselist\n";

while( $casedir = <CASELIST>) {
    chomp( $casedir);
    $casedir=~s/^\s*|\s*$//g;
    if( substr( $casedir, 0, 1) ne "/" ) {
        $casedir = "$pwd/$casedir";
    }
    print "- Testing case $casedir.\n";
    print SUMMARY "- Testing case $casedir.\n";
 
    if ( !( -d $casedir) ) {
        print "  Error: Cannot find case directory $casedir.\n";
        print SUMMARY "  Error: Cannot find case directory $casedir.\n";
        next;
    }

    @tmp = split( '/', $casedir);
    $case = $tmp[-1];

    $casein = "$casedir/$case.in";
    if ( !( -e $casein) ) {
        print "  Error: Cannot find case input file $casein.\n";
        print SUMMARY "  Error: Cannot find case input file $casein.\n";
        next;
    }

    $rundir = "$dir/$case";
    if( !mkdir( $rundir ) ) {
        print "  Error: Cannot create run directory $rundir.\n";
        print SUMMARY "  Error: Cannot create run directory $rundir.\n";
        next;
    }

    $cmd = "cd $rundir; $exe $casein $case.out | tee $case.log";
    print "  Execute command: $cmd.\n";
    print SUMMARY "  Execute command: $cmd.\n";

    $starttime = time();
    system( $cmd );
    if( $? == -1) {
        print "  Error: Failed to execute command $cmd.\n";
        print SUMMARY "  Error: Failed to execute command $cmd.\n";
        next;
    }
    $endtime = time();

    $runtime = $endtime - $starttime;
    print "  Run time: $runtime seconds.\n";
    print SUMMARY "  Run time: $runtime seconds.\n";

    $caseout = "$rundir/$case.out";
    if ( !( -e $caseout) ) {
        print "  Error: Cannot find case output file $caseout.\n";
        print SUMMARY "  Error: Cannot find case output file $caseout.\n";
        next;
    }

    $cmd = "/home/anlogic/script/score.pl $casein $caseout";
    system( "$cmd > tmp");
    if( $? != 0 ) {
        print "  Error: Failed to execute command $cmd.\n";
        print SUMMARY "  Error: Failed to execute command $cmd.\n";
        system( "rm tmp");
        next;
    }

    $line = `grep Score tmp`;
    @tmp = split( ' ', $line);
    $score = $tmp[2];
    
    system( "rm tmp");
    print "  Score: $score.\n";
    print SUMMARY "  Score: $score.\n";
}

close CASELIST;
close SUMMARY;

