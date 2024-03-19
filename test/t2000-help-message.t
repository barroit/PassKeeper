use v5.38;
use Test::More;
use IPC::Run 'run';

my @commands = ('count', 'create', 'delete', 'help', 'init', 'read', 'update', 'version');
my $general_message = qr/^usage: pk .*\n\n/;
my @cmd;
my $output;

foreach (@commands)
{
	@cmd = ($ENV{PKBIN}, 'help', $_);
	run \@cmd, '>', \$output;
	like($output, $general_message, "run pk help $_");

	@cmd = ($ENV{PKBIN}, $_, '-h');
	run \@cmd, '>', \$output;
	like($output, $general_message, "run pk $_ -h");
}

@cmd = ($ENV{PKBIN});
run \@cmd, '>', \$output;
like($output, $general_message, "run program without any arguments");

@cmd = ($ENV{PKBIN}, 'help');
run \@cmd, '>', \$output;
like($output, $general_message, "run help command without any arguments");

@cmd = ($ENV{PKBIN}, 'help', 'pk');
run \@cmd, '>', \$output;
like($output, $general_message, "request help for main command");

my $dummycmd = 'dummy command NEVER succeeds';
@cmd = ($ENV{PKBIN}, 'help', $dummycmd);
run \@cmd, '>', \$output, '2>&1';
is($output, "pk: '$dummycmd' is not a passkeeper command. See 'pk help pk'\n", "run help command with an unknown argument");

done_testing();