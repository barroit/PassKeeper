use v5.38;
use Test::More;
use Env qw(TEST_TMP_PREFIX PKBIN $PK_CRED_CC $PK_CRED_DB);
use IPC::Run 'run';

$PK_CRED_DB="$TEST_TMP_PREFIX/t3001-db";
$PK_CRED_CC="$TEST_TMP_PREFIX/t3001-ky";

my @args;
my @cmd;
my $output;

@args = (
	# [ 0, '--remember' ],
	# [ 0, '--kdf-algorithm', 'PBKDF2HMAC_SHA256' ],
	# [ 0, '--hmac-algorithm', 'HMAC_SHA1' ],
	# [ 0, '--cipher-compat', '3' ],
	# [ 0, '--page-size', '1024' ],
	# [ 0, '--kdf-iter', '5000' ],
	[ 1, '--remember', '--encrypt' ],
);

foreach (@args)
{
	my ($has_key_file, @rest) = @$_;
	@cmd = ($PKBIN, 'init', @rest);
	run \@cmd, '>', \$output;

	if ($has_key_file)
	{
		ok(-e $PK_CRED_CC, "key file exists");
		unlink($PK_CRED_CC);
	}
	else
	{
		ok(! -e $PK_CRED_CC, "key file not exists");
	}

	ok(-e $PK_CRED_DB, "db file exists");
	unlink($PK_CRED_DB);
}

@args = (
	[ 'kdf-algorithm', 'PBKDF2HMAC_SHAxxx', "'PBKDF2HMAC_SHAxxx' is not found in KDF algorithm list." ],
	[ 'hmac-algorithm', 'HMAC_SHAx', "'HMAC_SHAx' is not found in HMAC algorithm list." ],
	[ 'cipher-compat', '5', "Unknown cipher compatibility '5'." ],
	[ 'page-size', '1025', "Invalid page size '1025'." ],
);

foreach (@args)
{
	my ($opt, $val, $expect) = @$_;

	@cmd = ($PKBIN, 'init', "--$opt", $val, '--encrypt="some passphrases"');
	run \@cmd, '>', \$output, '2>&1';

	is($output, "error: $expect\n", "test $opt with value %val");
}

done_testing();
