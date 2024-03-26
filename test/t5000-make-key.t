use v5.38;
use Test::More;
use Env qw(PKBIN);
use IPC::Run 'run';

my @cmd;
my $output;

@cmd = ($PKBIN, 'makekey');
run \@cmd, '>', \$output;
like($output, qr/0x[A-F0-9]{64}/, "run makekey without any arguments (32 bytes key)");

@cmd = ($PKBIN, 'makekey', '--size', '5');
run \@cmd, '>', \$output;
like($output, qr/0x[A-F0-9]{10}/, "run makekey with binary key size 5");

my $outfile = "$ENV{TEST_TMP_PREFIX}/makekey.output";
@cmd = ($PKBIN, 'makekey', '--output', $outfile);
run \@cmd, '>', \$output;
@cmd = ('cat', $outfile);
run \@cmd, '>', \$output;
like($output, qr/0x[A-F0-9]{64}/, "run makekey with output to a special file");

done_testing();