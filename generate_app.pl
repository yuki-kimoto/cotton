use strict;
use warnings;

my ($app_name) = @ARGV;

die unless defined $app_name;

# Create SPVM module file
my $app_spvm_module_file = "lib/$app_name.spvm";
my $app_spvm_module_content = <<"EOS";
package $app_name {
  sub main : int (\$args : string[]) {
    call_win_main();
  }

  native sub call_win_main : void ();
}
EOS
create_file($app_spvm_module_file, $app_spvm_module_content);

# Create SPVM native config file
my $app_spvm_native_config_file = "lib/$app_name.config";
my $app_spvm_native_config_content = <<'EOS';
use strict;
use warnings;

use SPVM::Builder::Config;

my $bconf = SPVM::Builder::Config->new_c99;

$bconf;
EOS
create_file($app_spvm_native_config_file, $app_spvm_native_config_content);

# Create SPVM native file
my $app_spvm_native_file = "lib/$app_name.c";
my $app_spvm_native_content = <<"EOS";
#include <spvm_native.h>

#include <windows.h>

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, int showCmd)
{
    MessageBox(NULL, "Hello world.", "Message", MB_OK);

    return 0;
}

int32_t SPNATIVE__${app_name}__call_win_main(SPVM_ENV* env, SPVM_VALUE* args) {
  (void)env;
  (void)args;
  
  HINSTANCE hInst = GetModuleHandle(NULL) ;
  
  WinMain(hInst, NULL, NULL, SW_SHOWNORMAL);

  return SPVM_SUCCESS;
}
EOS
create_file($app_spvm_native_file, $app_spvm_native_content);

# Print spvmcc command
my $spvmcc_cmd = "spvmcc -I lib -o $app_name $app_name";
print "$spvmcc_cmd\n";

sub create_file {
  my ($file, $content) = @_;
  
  open my $fh, '>', $file
    or die "Can't open file: $!";
    
  print $fh $content;
}
