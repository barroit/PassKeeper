use v5.38;
use Test::More;
use Env qw(BRTOOL_TEST_PREFIX PKBIN);
use IPC::Run 'run';

my %pathnames = (
	# "$BRTOOL_TEST_PREFIX/dummy/root.file" => [
	# 	qr/^error: access denied by '.+\/dummy\/root\.file'$/,
	# 	'reading a file without permission (absolute path)',
	# ],
	"BRTOOL_TEST_PREFIX/dummy/unexists.file" => [
		qr/^error: '.+\/dummy\/unexists\.file' did not match any files$/,
		'reading a file that does not exist (absolute path)',
	],
	# "$BRTOOL_TEST_PREFIX/dummy/user.dir" => [
	# 	qr/^error: '.+\/dummy\/user\.dir' is not a regular file$/,
	# 	'reading a directory (absolute path)',
	# ],
	# "dummy/root.file" => [
	# 	qr/^error: access denied by '.+\/dummy\/root\.file'$/,
	# 	'reading a file without permission (relative path)',
	# ],
	"./dummy/unexists.file" => [
		qr/^error: '.+\/dummy\/unexists\.file' did not match any files$/,
		'reading a file that does not exist (relative path)',
	],
	# "../$BRTOOL_TEST_PREFIX/dummy/user.dir" => [
	# 	qr/^error: '.+\/dummy\/user\.dir' is not a regular file$/,
	# 	'reading a directory (relative path)',
	# ],
);

while(my($filename, $rest) = each %pathnames)
{
	my ($regex, $description) = @$rest;

	my @cmd = ($PKBIN, '--db-path', "$filename");
	run \@cmd, '>', \my $output, '2>&1';

	like($output, $regex, $description);
}

done_testing();