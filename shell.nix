{ pkgs ? import <nixpkgs> {
	config.allowUnfree = true;
} }:
pkgs.mkShell {
	buildInputs = with pkgs; [
		stm32cubemx
		gcc-arm-embedded
		stlink
		screen
		minicom
		picocom
    ];

	shellHook = ''
		export NIXPKGS_ALLOW_UNFREE=1
		fish
	'';
}
