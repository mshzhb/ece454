my $output = $ARGV[0];
system("touch " . $output);

my $cmd = "/usr/bin/time -f 'randtrack_1_50 %e' -a -o ". $output." ./randtrack 1 50";
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

my $samples = 50;
while ($samples < 101) {
	my $counter = 1;
	while ($counter < 5){
		$cmd = "/usr/bin/time -f 'gl_".$counter."_".$samples." %e' -a -o ". $output." ./randtrack_global_lock ".$counter." ".$samples." ";
		print ($cmd);
		my $MAX_RUNS = 5;
		for ($i = 0; $i < $MAX_RUNS; $i++){
			print("run ".$i." of ".$MAX_RUNS."\n");
			system($cmd);
		}

		$cmd = "/usr/bin/time -f 'tm_".$counter."_".$samples." %e' -a -o ". $output." ./randtrack_tm ".$counter." ".$samples." ";
		print ($cmd);
		my $MAX_RUNS = 5;
		for ($i = 0; $i < $MAX_RUNS; $i++){
			print("run ".$i." of ".$MAX_RUNS."\n");
			system($cmd);
		}

		$cmd = "/usr/bin/time -f 'll_".$counter."_".$samples." %e' -a -o ". $output." ./randtrack_list_lock ".$counter." ".$samples." ";
		print ($cmd);
		my $MAX_RUNS = 5;
		for ($i = 0; $i < $MAX_RUNS; $i++){
			print("run ".$i." of ".$MAX_RUNS."\n");
			system($cmd);
		}

		$cmd = "/usr/bin/time -f 'el_".$counter."_".$samples." %e' -a -o ". $output." ./randtrack_element_lock ".$counter." ".$samples." ";
		print ($cmd);
		my $MAX_RUNS = 5;
		for ($i = 0; $i < $MAX_RUNS; $i++){
			print("run ".$i." of ".$MAX_RUNS."\n");
			system($cmd);
		}
		$counter = $counter + $counter;
	}
	$samples = $samples + $samples;
}




