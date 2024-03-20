use v5.38;
use Test::More;
use IPC::Run 'run';

my @data = (
	['', '.'],
	['.', '.'],
	['..', '.'],
	['not.dir', '.'],
	['/', '/'],
	['usr/', '.'],
	['usr/bin', 'usr'],
	['/usr/', '/'],
	['/usr/lib', '/usr'],
);

my $input;
my $expect;

my @cmd;
my $output;

my $bin = "$ENV{TEST_BUILD_PREFIX}/t1000-pk_dirname";

foreach (@data)
{
	($input, $expect) = @$_;
	@cmd = ($bin, $input);
	run \@cmd, '>', \$output, '2>&1';
	is($output, $expect, "test dirname with input '$input'");
}

@cmd = ($bin);
run \@cmd, '>', \$output;
is($output, '.', "test dirname without input");

done_testing();