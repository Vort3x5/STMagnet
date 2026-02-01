{ pkgs ? import <nixpkgs> {
	config.allowUnfree = true;
} }:
pkgs.mkShell {
	buildInputs = with pkgs; [
		stm32cubemx
		gcc-arm-embedded
		stlink
		picocom
    ];

	shellHook = ''
		fish
	'';
}
