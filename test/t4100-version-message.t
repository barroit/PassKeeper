use v5.38;
use Test::More;
use Env qw(PKBIN);
use IPC::Run 'run';

my @cmd;
my $output;

# ensure every project information are set correctly
# if this test failed, check that the environment variable exported by
# cmake or if the project config header is set properly

@cmd = ($PKBIN, 'version');
run \@cmd, '>', \$output;
like($output, qr/^pk \(.+\).*\d\.\d+\.\d\nCopyr.+\n.+tten by .+<\S+com>$/s, "run version command without any arguments");

@cmd = ($PKBIN, 'version', '--build-options');
run \@cmd, '>', \$output;
like($output, qr/^pk \(.+\).*\d\.\d+\.\d\nCopyr.+\n.+ure: \S+\n.*mmit: \w+\n.+tten by .+<\S+com>$/s, "run version command with option --build-options");

done_testing();
