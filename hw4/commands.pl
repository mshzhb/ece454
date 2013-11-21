my $output = $ARGV[0];
system("touch " . $output);

my $cmd = "/usr/bin/time -f '\\t%e' -a -o ". $output." ./randtrack 1 50";
print ($cmd);

my $MAX_RUNS = 5;
for ($i = 0; $i < $MAX_RUNS; $i++){
	print("run ".$i." of ".$MAX_RUNS."\n");
	system($cmd);
}


