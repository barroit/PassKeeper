use v5.38;
use Test::More;
use Env qw(TEST_TMP_PREFIX PKBIN $PK_CRED_KY $PK_CRED_DB);
use autodie;
use IPC::Run 'run';

$PK_CRED_DB="$TEST_TMP_PREFIX/t3000-db";
$PK_CRED_KY="$TEST_TMP_PREFIX/t3000-ky";

my @args;
my $arg;
my @cmd;
my $output;

# @args = ("x''", "x", "x'98483C6EB40B6C31A448C22A66DED3B5E5E8D5119CAC8327B655C8B5C4836481010101010101010101010101010101011'");

# foreach (@args)
# {
# 	@cmd = ($PKBIN, "init", "--encrypt=$_");
# 	run \@cmd, '>', \$output, '2>&1';

# 	is($output, "error: invalid key length \"$_\"\n", "encryption key has incorrect length");
# }

@args = ("x'2DD29CA851E7B56E4697B0E1F08507293D761A05CEGD1B628663F411A8086D99'", "x'98483C6EB40B6C31A448C22A66DED3BKE5E8D5119CAC8327B655C8B5C483648101010101010101010101010101010101'");

foreach (@args)
{
	@cmd = ($PKBIN, "init", "--encrypt=$_");
	run \@cmd, '>', \$output, '2>&1';

	is($output, "error: Blob key \"$_\" contains invalid char.\n", "encryption key has invalid char");
}

$arg = "x'98483C6EB40B6C31A448C22A66DED3B5E5E8D5119CAC8327B655C8B5C483648101010101010101010101010101010102'";

@cmd = ($PKBIN, "init", "--encrypt=$arg");
run \@cmd, '>', \$output, '2>&1';
is($output, "error: Blob key \"$arg\" contains invalid salt.\n", "encryption key has incorrect salt value");

@cmd = ($PKBIN, 'init', '--encrypt=');
run \@cmd, '>', \$output, '2>&1';
is($output, "error: Blank passphrase is not allowed.\n", "encryption passphrase is empty");

@args = (
	[ 0, '', 1 ],
	[ 1, '', 0],
	[ 0, 'miku', 0 ],
	[ 1, "x'98483C6EB40B6C31A448C22A66DED3B5E5E8D5119CAC8327B655C8B5C4836481'", 0 ],
	[ 0, "x'98483C6EB40B6C31A448C22A66DED3B5E5E8D5119CAC8327B655C8B5C4836481'", 1 ],
);

foreach (@args)
{
	my ($has_key_file, $key, $no_remember) = @$_;

	@cmd = ($PKBIN, 'init', $key ? "--encrypt=$key" : "--encrypt");
	if ($no_remember)
	{
		push @cmd, '--no-remember';
	}

	run \@cmd, '>', \$output;

	if ($has_key_file)
	{
		ok(-e $PK_CRED_KY, "key file exists");
		unlink($PK_CRED_KY);
	}
	else
	{
		ok(! -e $PK_CRED_KY, "key file not exists");
	}

	ok(-e $PK_CRED_DB, "db file exists");
	unlink($PK_CRED_DB);
}

done_testing();
