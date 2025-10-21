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
		texlive.combined.scheme-full
		evince
    ];

	shellHook = ''
		fish
	'';
}
