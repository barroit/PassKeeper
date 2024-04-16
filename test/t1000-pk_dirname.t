use v5.38;
use Test::More;
use Env qw(TEST_BUILD_PREFIX);
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

my @cmd;
my $output;
my $PKBIN = "$TEST_BUILD_PREFIX/t1000-pk_dirname";

foreach (@data)
{
	my ($input, $expect) = @$_;
	@cmd = ($PKBIN, $input);
	run \@cmd, '>', \$output, '2>&1';
	is($output, $expect, "test dirname with input '$input'");
}

@cmd = ($PKBIN);
run \@cmd, '>', \$output;
is($output, '.', "test dirname without input");

done_testing();
