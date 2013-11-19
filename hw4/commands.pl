my $output = $ARGV[0];
system("touch " . $output);

#my $cmd = "/usr/bin/time -a -o ". $output." vpr iir1.map4.latren.net k4-n10.xml place.out route.out -nodisp -place_only -seed 0";
my $cmd = "/usr/bin/time -a -o ". $output." make";
print ($cmd);

my $MAX_RUNS = 5;
for ($i = 0; $i < $MAX_RUNS; $i++){
	print("run ".$i." of ".$MAX_RUNS."\n");
	system("make clean");
	system($cmd);
}


