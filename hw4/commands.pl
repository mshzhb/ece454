my $output = $ARGV[0];
system("touch " . $output);

my $cmd = "/usr/bin/time -f 'randtrack_1_50 %e' -a -o ". $output." ./randtrack 1 50";
print ($cmd);

my $MAX_RUNS = 5;
for ($i = 0; $i < $MAX_RUNS; $i++){
	print("run ".$i." of ".$MAX_RUNS."\n");
	system($cmd);
}

$cmd = "/usr/bin/time -f 'gl_1_50 %e' -a -o ". $output." ./randtrack_global_lock 1 50";
print ($cmd);

my $MAX_RUNS = 5;
for ($i = 0; $i < $MAX_RUNS; $i++){
	print("run ".$i." of ".$MAX_RUNS."\n");
	system($cmd);
}

$cmd = "/usr/bin/time -f 'tm_1_50 %e' -a -o ". $output." ./randtrack_tm 1 50";
print ($cmd);

my $MAX_RUNS = 5;
for ($i = 0; $i < $MAX_RUNS; $i++){
	print("run ".$i." of ".$MAX_RUNS."\n");
	system($cmd);
}

$cmd = "/usr/bin/time -f 'll_1_50 %e' -a -o ". $output." ./randtrack_list_lock 1 50";
print ($cmd);

my $MAX_RUNS = 5;
for ($i = 0; $i < $MAX_RUNS; $i++){
	print("run ".$i." of ".$MAX_RUNS."\n");
	system($cmd);
}

$cmd = "/usr/bin/time -f 'el_1_50 %e' -a -o ". $output." ./randtrack_element_lock 1 50";
print ($cmd);

my $MAX_RUNS = 5;
for ($i = 0; $i < $MAX_RUNS; $i++){
	print("run ".$i." of ".$MAX_RUNS."\n");
	system($cmd);
}





my $cmd = "/usr/bin/time -f 'randtrack_1_100 %e' -a -o ". $output." ./randtrack 1 100";
print ($cmd);

my $MAX_RUNS = 5;
for ($i = 0; $i < $MAX_RUNS; $i++){
	print("run ".$i." of ".$MAX_RUNS."\n");
	system($cmd);
}

$cmd = "/usr/bin/time -f 'gl_1_100 %e' -a -o ". $output." ./randtrack_global_lock 1 100";
print ($cmd);

my $MAX_RUNS = 5;
for ($i = 0; $i < $MAX_RUNS; $i++){
	print("run ".$i." of ".$MAX_RUNS."\n");
	system($cmd);
}

$cmd = "/usr/bin/time -f 'tm_1_100 %e' -a -o ". $output." ./randtrack_tm 1 100";
print ($cmd);

my $MAX_RUNS = 5;
for ($i = 0; $i < $MAX_RUNS; $i++){
	print("run ".$i." of ".$MAX_RUNS."\n");
	system($cmd);
}

$cmd = "/usr/bin/time -f 'll_1_100 %e' -a -o ". $output." ./randtrack_list_lock 1 100";
print ($cmd);

my $MAX_RUNS = 5;
for ($i = 0; $i < $MAX_RUNS; $i++){
	print("run ".$i." of ".$MAX_RUNS."\n");
	system($cmd);
}

$cmd = "/usr/bin/time -f 'el_1_100 %e' -a -o ". $output." ./randtrack_element_lock 1 100";
print ($cmd);

my $MAX_RUNS = 5;
for ($i = 0; $i < $MAX_RUNS; $i++){
	print("run ".$i." of ".$MAX_RUNS."\n");
	system($cmd);
}