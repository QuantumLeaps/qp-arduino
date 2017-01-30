##############################################################################
# Product: TCL script for building Arduino sketches
# Last updated for: Arduino 1.6.8
# Date of the Last Update: 2016-04-02
#
#                    Q u a n t u m     L e a P s
#                    ---------------------------
#                    innovating embedded systems
#
# Copyright (C) Quantum Leaps, LLC. All rights reserved.
#
# This program is open source software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Contact information:
# Web:     http://www.state-machine.com
# Support: http://sourceforge.net/p/qpc/discussion/668726
##############################################################################
#
# usage
# $tclsh avrmake.tcl <PROJECT> "<LIBS>" ["<DEFINES>"]
#
# NOTE: Typically, you don't need to modify this script to build your Arduino
# sketches. All information is provided in the parameters.
#

if { $argc < 2 } {
    puts "usage: tclsh avrmake.tcl <PROJECT> \"<LIBS>\"" \[\"<DEFINES>\"\]
    puts "example: tclsh avrmake.tcl blinky atmega328p 16 \"qpn_avr\"" \"QK_PREEMPTIVE\"

    exit 1
}

set PROJECT            [lindex $argv 0]
set ARDUINO_LIBS       [lindex $argv 1]
set ARDUINO_DEFS       [lindex $argv 2]

#=============================================================================
# change the provided names to DOS-convention to avoid spaces and special
# characters in the path names...
set ARDUINO_HOME       \
    [file attributes [string trim $env(ARDUINO_HOME) \"] -shortname]
set ARDUINO_SKETCHBOOK \
    [file attributes [string trim $env(ARDUINO_SKETCHBOOK) \"] -shortname]

set BOARD_MCU          $env(BOARD_MCU)
set BOARD_VARIANT      $env(BOARD_VARIANT)
set BOARD_F_CPU        [expr $env(BOARD_F_MHZ) * 1000000]L
set BOARD_CORE         "arduino"

# set the CYGWIN environment variable to avoid warnings about DOS file names
set env(CYGWIN) nodosfilewarning

set AVR_TOOLS "$ARDUINO_HOME/hardware/tools/avr/bin"
set CC   "$AVR_TOOLS/avr-gcc"
set CXX  "$AVR_TOOLS/avr-g++"
set AR   "$AVR_TOOLS/avr-ar"
set LINK "$AVR_TOOLS/avr-gcc"

set OBJCOPY "$AVR_TOOLS/avr-objcopy"
set OBJDUMP "$AVR_TOOLS/avr-objdump"

#.............................................................................
# directories
#

set BIN_DIR bin
set LIB_DIR lib

#.............................................................................
# project source files
#
set CXX_FILES [glob -nocomplain *.cpp]
set CXX_FILES [concat $CXX_FILES [glob -nocomplain *.ino]]
set C_FILES   [glob -nocomplain *.c]

#.............................................................................
# compile and link options
#
set LIB_CORE_DIR    "$ARDUINO_HOME/hardware/arduino/avr/cores"
set LIB_BUILTIN_DIR "$ARDUINO_HOME/hardware/arduino/avr/libraries"
set LIB_USER_DIR    "$ARDUINO_SKETCHBOOK/libraries"

# Place -I options here
set INCLUDES "-I. -I$LIB_CORE_DIR/$BOARD_CORE -I$LIB_CORE_DIR/../variants/$BOARD_VARIANT"
set DEFINES  "-DF_CPU=$BOARD_F_CPU"
foreach def $ARDUINO_DEFS {
    append DEFINES " -D" $def
}

# Compiler flag to set the C Standard level.
set CXXFLAGS  "-x c++ -mmcu=$BOARD_MCU -Os -Wall -Wno-attributes \
 -fno-exceptions -ffunction-sections -fdata-sections"
set CFLAGS    "-mmcu=$BOARD_MCU -Os -Wall -Wno-attributes \
 -ffunction-sections -fdata-sections -std=gnu99"
set LDFLAGS   "-Wl,-Map,$BIN_DIR/$PROJECT.map,--cref"

#=============================================================================
# assert a condition
proc make.assert { cond msg } {
    if { $cond == 0 } {
        global PROJECT BIN_DIR PATH

        # cleanup the project files, so that it will be intentionally broken
        make.delete $BIN_DIR $PROJECT.*

        # restore the orignal PATH environment variable
        set env(PATH) $PATH

        # print the message and exit with an error
        puts stderr "\n====>ERROR: $msg"
        exit 1
    }
}
#.............................................................................
# execute a build command
#
proc make.exec args {
    set cmd [join $args]
    puts $cmd
    catch {eval exec -- $cmd} msg
    if {[string length $msg]} {
        puts $msg
    }
    global EXEC
    incr EXEC
}
#.............................................................................
# return 1 if any file in the dependency list is newer than a given file
#
proc make.filesAreNewer {depList f} {
    foreach dep $depList {
        if {![file exists $dep] || ![file exists $f]} {
            return 1
        }
        if {[file mtime $dep] > [file mtime $f]} {
            return 1
        }
    }
    return 0
}
#.............................................................................
proc make.fileChangeExt { file ext } {
    return [file tail [file rootname $file]]$ext
}
#.............................................................................
# generate TCL list of dependencies from the GNU-type dependency (.d) file
#
proc make.genDepList {depFile} {
    set fileList {}
    if { [catch {open $depFile "r"} fid] } {
        make.assert 0 $fid
    }
    set contentList [split $[read -nonewline $fid] "\n "]
    catch { close $fid }

    foreach str $contentList {
        if { $str != "\\" && $str != " "} {
            set $str [file normalize $str]
            if { [file exists $str] } {
                lappend fileList [file normalize $str]
            }
        }
    }
    return $fileList
}
#.............................................................................
proc make.delete { dir pattern } {
    set fileList [glob -nocomplain -directory $dir $pattern]
    foreach f $fileList {
        file delete $f
    }
}

#=============================================================================
proc makeCxxDeps { srcList depDir } {
    foreach f $srcList {
        set depFile $depDir/[make.fileChangeExt $f ".d"]
        if { [make.filesAreNewer $f $depFile] } {
            global CXX INCLUDES DEFINES
            make.exec $CXX -x c++ -MM -MT [make.fileChangeExt $f ".o"] \
                      $INCLUDES $DEFINES $f > $depFile
        }
    }
}
#.............................................................................
proc makeCDeps { srcList depDir } {
    foreach f $srcList {
        set depFile $depDir/[make.fileChangeExt $f ".d"]
        if { [make.filesAreNewer $f $depFile] } {
            global CC INCLUDES DEFINES
            make.exec $CC -MM -MT [make.fileChangeExt $f ".o"] \
                      $INCLUDES $DEFINES $f > $depFile
        }
    }
}
#.............................................................................
proc makeCxxObjs { srcList depDir binDir } {
    foreach f $srcList {
        set objFile $binDir/[make.fileChangeExt $f ".o"]
        set depList [make.genDepList $depDir/[make.fileChangeExt $f ".d"]]
        if { [make.filesAreNewer $depList $objFile] } {
            global CXX CXXFLAGS INCLUDES DEFINES
            file delete $objFile
            make.exec $CXX -c $CXXFLAGS $INCLUDES $DEFINES $f -o $objFile
            make.assert [file exists $objFile] "'$f' failed to compile"
        }
    }
}
#.............................................................................
proc makeCObjs { srcList depDir binDir } {
    foreach f $srcList {
        set objFile $binDir/[make.fileChangeExt $f ".o"]
        set depList [make.genDepList $depDir/[make.fileChangeExt $f ".d"]]
        if { [make.filesAreNewer $depList $objFile] } {
            global CC CFLAGS INCLUDES DEFINES
            file delete $objFile
            make.exec $CC -c $CFLAGS $INCLUDES $DEFINES $f -o $objFile
            make.assert [file exists $objFile] "'$f' failed to compile"
        }
    }
}
#.............................................................................
proc makeLib { libName libDir outDir } {
    set lib $libDir/$libName
    set libFile $outDir/$libName.a
    if { ![file exists $lib] } {
        puts "library not found $lib"
        exit 1
    }
    if { [file exists $libFile] } {
        return $libFile
    }
    set cxxFiles [glob -nocomplain -directory $lib "*.cpp"]
    set cxxFiles [concat $cxxFiles [glob -nocomplain -directory $lib "*.ino"]]
    set cFiles   [glob -nocomplain -directory $lib "*.c"]
    set utility  ""
    if { [file exists $lib/utility] } {
        set cxxFiles [concat $cxxFiles [glob -nocomplain \
                      -directory $lib/utility "*.cpp"]]
        set cxxFiles [concat $cxxFiles [glob -nocomplain \
                      -directory $lib/utility "*.ino"]]
        set cFiles   [concat $cFiles [glob -nocomplain \
                      -directory $lib/utility "*.c"]]
        set utility "-I$lib/utility"
    }
    foreach f $cxxFiles {
        set objFile $outDir/[make.fileChangeExt $f ".o"]
        global AR CXX CXXFLAGS LIB_INCLUDES DEFINES
        file delete $objFile
        make.exec $CXX -c $CXXFLAGS $LIB_INCLUDES $utility $DEFINES $f -o $objFile
        make.assert [file exists $objFile] "'$f' failed to compile"
        make.exec $AR rcs $libFile $objFile
        file delete $objFile
    }
    foreach f $cFiles {
        set objFile $outDir/[make.fileChangeExt $f ".o"]
        global AR CC CFLAGS LIB_INCLUDES DEFINES
        file delete $objFile
        make.exec $CC -c $CFLAGS $LIB_INCLUDES $utility $DEFINES $f -o $objFile
        make.assert [file exists $objFile] "'$f' failed to compile"
        make.exec $AR rcs $libFile $objFile
        file delete $objFile
    }
    return $libFile
}
#.............................................................................
proc makeElf { projName srcList libList binDir } {
    set objList {}
    foreach f $srcList {
        set objFile $binDir/[make.fileChangeExt $f ".o"]
        lappend objList $objFile
    }
    set elfFile $binDir/$projName.elf
    if { [make.filesAreNewer $objList $elfFile] } {
        global LINK CFLAGS LIB_DIR LDFLAGS
        file delete $elfFile
        make.exec $LINK $CFLAGS [join $objList] [join $libList] \
                  -lm --output $elfFile $LDFLAGS
        make.assert [file exists $elfFile] "'$elfFile' failed to link"
    }
}
#.............................................................................
proc makeHex { projName binDir } {
    set elfFile $binDir/$projName.elf
    set hexFile $binDir/$projName.hex
    if { [make.filesAreNewer $elfFile $hexFile ] } {
        global OBJCOPY
        file delete $hexFile
        make.exec $OBJCOPY -O ihex -R .eeprom $elfFile $hexFile
        make.assert [file exists $elfFile] "'$hexFile' could not be created"
    }
}
#.............................................................................
proc makeEep { projName binDir } {
    set elfFile $binDir/$projName.elf
    set eepFile $binDir/$projName.eep
    if { [make.filesAreNewer $elfFile $eepFile ] } {
        global OBJCOPY
        file delete $eepFile
        make.exec $OBJCOPY -O ihex -j .eeprom \
                  --set-section-flags=.eeprom=alloc,load \
                  --no-change-warnings --change-section-lma .eeprom=0 \
                  $elfFile $eepFile
        make.assert [file exists $elfFile] "'$eepFile' could not be created"
    }
}

#-----------------------------------------------------------------------------
set EXEC 0  ;# number of build steps actually exectued

# store the original PATH and change the PATH to $ARDUINO_HOME
# NOTE: $ARDUINO_HOME must be in the PATH to find CygWin DLLs
#
set PATH      $env(PATH)
set env(PATH) $ARDUINO_HOME

if { ![file exists $BIN_DIR] } {
    file mkdir $BIN_DIR
}
if { ![file exists $LIB_DIR] } {
    file mkdir $LIB_DIR
}
set LIB_INCLUDES $INCLUDES
set builtinLibs [glob -nocomplain $LIB_BUILTIN_DIR/*]
foreach l $builtinLibs {
    append LIB_INCLUDES " -I$l"
}

# bulid the Arduino core library...
set coreLib [makeLib $BOARD_CORE "$LIB_CORE_DIR" $LIB_DIR]
set libList $coreLib

# bulid all additional library listed explicitly...
foreach l $ARDUINO_LIBS {
    set loc_dir "$LIB_BUILTIN_DIR"      ;# assume it is a builtin library
    if { ![file exists $loc_dir/$l] } {
        set loc_dir "$LIB_USER_DIR"     ;# try the user library
    }
    if { [file exists $loc_dir/$l] } {
        set lib [makeLib $l $loc_dir $LIB_DIR] ;# build the library...
        append INCLUDES " -I$loc_dir/$l"
        lappend libList $lib
    }
}
# append the Arduino core library again, to resolve all dependencies
# on the core that might appear in all the other libraries
#
lappend libList $coreLib

makeCxxDeps $CXX_FILES $BIN_DIR
makeCDeps   $C_FILES   $BIN_DIR
makeCxxObjs $CXX_FILES $BIN_DIR $BIN_DIR
makeCObjs   $C_FILES   $BIN_DIR $BIN_DIR
makeElf     $PROJECT   [concat $CXX_FILES $C_FILES] $libList $BIN_DIR
makeHex     $PROJECT   $BIN_DIR
makeEep     $PROJECT   $BIN_DIR

if { $EXEC == 0 } { ;# number of build steps exectued is still zero?
    puts "\n====> OK (up to date)"
} else {
    puts "\n====> OK"
}

# restore the original PATH
set env(PATH) $PATH



