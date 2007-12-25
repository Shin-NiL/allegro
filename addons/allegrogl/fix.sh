#!/bin/sh

proc_unix()
{
echo Configuring AllegroGL to build on Unix...

rm -f makefile
mkdir -p obj/unix/debug obj/unix/release lib/unix

}

proc_fix()
{
echo "Configuring AllegroGL for $1..."
echo "# generated by fix.sh" > makefile
case "$1" in
	MSVC )
		echo "Notice: Because no version was specified, MSVC 6 has been chosen."
		echo ""
		echo "If you are using a newer version, you should use 'msvc7' or 'msvc8' instead."
		echo "msvc7 should be used for MSVC .NET or MSVC .NET 2003"
		echo "msvc8 should be used for MSVC .NET 2005"
		echo ""
		echo "COMPILER_MSVC6 = 1" >> makefile;;
	MSVC6) echo "COMPILER_MSVC6 = 1" >> makefile;;
	MSVC7) echo "COMPILER_MSVC7 = 1" >> makefile;;
	MSVC8) echo "COMPILER_MSVC8 = 1" >> makefile;;
esac
echo "include make/$2" >> makefile
echo ""
echo "Done!"
echo ""
}

display_help()
{
echo "Compilation target adjustment."
echo "  Usage: fix <platform> [--dtou|--utod|--quick]"
echo ""
echo "  <platform> is one of: djgpp, mingw32, msvc6, msvc7, msvc8, unix, macosx"
echo ""
echo "  --dtou converts from DOS/Win32 format to Unix"
echo "  --utod converts from Unix format to DOS/Win32"
echo "  --quick does no line ending conversion"
echo "  If no parameter is specified --quick is assumed"
echo ""
echo "  Example: fix unix --dtou"
echo ""
}

proc_filelist()
{
	AL_FILELIST=`find . -type f -not -path "*.svn*" "(" \
		-name "*.c" -o -name "*.cfg" -o -name "*.cpp" -o -name "*.dep" -o \
		-name "*.h" -o -name "*.hin" -o -name "*.in" -o -name "*.inc" -o \
		-name "*.m4" -o -name "*.mft" -o -name "*.s" -o -name "config*" -o \
		-name "*.spec" -o -name "*.pl" -o -name "*.txt" -o -name "*._tx" -o \
		-name "makefile.*" -o -name "readme.*" -o -name "changelog" \
		")"`

	# touch unix shell scripts?
	if [ "$1" != "omit_sh" ]; then
		AL_FILELIST="$AL_FILELIST `find . -type f -name '*.sh'`"
	fi

	# touch DOS batch files?
	if [ "$1" != "omit_bat" ]; then
		AL_FILELIST="$AL_FILELIST `find . -type f -name '*.bat'`"
	fi
}

proc_dtou()
{
	echo "Converting files from DOS/Win32 to Unix ..."
	proc_filelist "omit_bat"
	for file in $AL_FILELIST; do
		echo "$file"
		sh misc/dtou.sh $file;
	done
}

proc_utod()
{
	echo "Converting files from Unix to DOS/Win32 ..."
	proc_filelist "omit_sh"
	for file in $AL_FILELIST; do
		echo "$file"
		sh misc/utod.sh $file;
	done
}


if [ -z "$1" ]; then
	display_help
	exit 0
fi


case "$1" in
	djgpp	) proc_fix "DJGPP" "makefile.dj";;
	mingw	) proc_fix "Mingw32" "makefile.mgw";;
	mingw32	) proc_fix "Mingw32" "makefile.mgw";;
	msvc	) proc_fix "MSVC" "makefile.vc";;
	msvc6	) proc_fix "MSVC6" "makefile.vc";;
	msvc7	) proc_fix "MSVC7" "makefile.vc";;
	msvc8	) proc_fix "MSVC8" "makefile.vc";;
	unix	) proc_unix;;
	macosx  ) proc_fix "MacOS X" "makefile.osx";;
	*	) echo "Platform not supported by AllegroGL."
esac

case "$2" in
	--utod	) proc_utod;;
	--dtou	) proc_dtou;;
	*	) ;;
esac

