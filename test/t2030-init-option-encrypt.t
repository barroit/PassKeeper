use v5.38;
use Test::More;
use IPC::Run 'run';

my @args;
my @cmd;
my $output;

@args = ("x''", "x", "x'98483C6EB40B6C31A448C22A66DED3B5E5E8D5119CAC8327B655C8B5C4836481010101010101010101010101010101011'");

foreach (@args)
{
	@cmd = ($ENV{PKBIN}, "init", "--encrypt=$_");
	run \@cmd, '>', \$output, '2>&1';

	is($output, "error: invalid key length \"$_\"\n", "command 'init' with incorrect encrypt key (incorrect length)");
}

@args = ("x'2DD29CA851E7B56E4697B0E1F08507293D761A05CEGD1B628663F411A8086D99'", "x'98483C6EB40B6C31A448C22A66DED3BKE5E8D5119CAC8327B655C8B5C483648101010101010101010101010101010101'");

foreach (@args)
{
	@cmd = ($ENV{PKBIN}, "init", "--encrypt=$_");
	run \@cmd, '>', \$output, '2>&1';

	is($output, "error: key contains invalid char \"$_\"\n", "command 'init' with incorrect encrypt key (invalid char)");
}

@args = ("x'98483C6EB40B6C31A448C22A66DED3B5E5E8D5119CAC8327B655C8B5C483648101010101010101010101010101010102'");

foreach (@args)
{
	@cmd = ($ENV{PKBIN}, "init", "--encrypt=$_");
	run \@cmd, '>', \$output, '2>&1';

	is($output, "error: invalid key salt \"$_\"\n", "command 'init' with incorrect encrypt key (incorrect salt value)");
}

done_testing();