{
  description = "DBMS, gcc kompilator, gnumake dla makefile, readline biblioteka dla historii komend użytkownika, nlohmann_json biblioteka do pracy z json. openssl do enkrypcji haseł. gdb do debugowania seg faultów.";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/24.05";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, ... }@inputs: inputs.utils.lib.eachSystem [
    "x86_64-linux" "i686-linux" "aarch64-linux" "x86_64-darwin"
  ] (system: let
    pkgs = import nixpkgs {
      inherit system;
      overlays = [];
      # config.allowUnfree = true;
    };
  in {
    devShells.default = pkgs.mkShell rec {
      name = "dbms";

      packages = with pkgs; [
        gcc
        gnumake
        gdb

        readline
        nlohmann_json
        openssl
	      fmt
      ];
            shellHook = ''
        echo "
        -------------------------------------------------------"
        echo "|  The development environment has been activated.    |"
        echo "-------------------------------------------------------
        "
        alias make="clear; time make"
      '';
      };
    packages.default = pkgs.callPackage ./default.nix {};
  });
}
