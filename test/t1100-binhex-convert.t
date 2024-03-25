use v5.38;
use Test::More;
use IPC::Run 'run';

my @cmd;
my $output;
my @hexdata = ('39', '0A5F', '2235C276DF3F483F43', '0xE5E394C907B4D8857CCFE1C39385A5124E0AD946ACB9A4B9D499A363FF4318D2');
my $bin = "$ENV{TEST_BUILD_PREFIX}/t1100-binhex-convert";

foreach (@hexdata)
{
	@cmd = ($bin, $_);
	run \@cmd, '>', \$output;
	is($output, $output, "test bin/hex convert with value $_");
}

done_testing();