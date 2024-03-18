use v5.38;
use Test::More tests => 1, qw/modern/;
use IPC::Run 'run';

my @command = ($ENV{PKBIN}, 'create', '--db-path', "$ENV{TEST_PREFIX}/dummy/root.file");
my $output;

run \@command, '>', \$output, '2>&1';

like($output, qr/^error: access denied by '.+?\/dummy\/root\.file'$/, 'reading a file without permission');