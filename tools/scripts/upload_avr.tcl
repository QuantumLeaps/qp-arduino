
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

set PROJECT            $env(PROJECT)

set BOARD_MCU          $env(BOARD_MCU)
set BOARD_COM          $env(BOARD_COM)
set BOARD_BAUD         $env(BOARD_BAUD)

set ARDUINO_HOME       $env(ARDUINO_HOME)
set ARDUINO_SKETCHBOOK $env(ARDUINO_SKETCHBOOK)

set AVR_TOOLS          $env(AVR_TOOLS)
set AVRDUDE            "$AVR_TOOLS/avrdude"

switch $tcl_platform(os) {
    "Darwin" {
        make.exec $AVRDUDE -p $BOARD_MCU -c arduino -P $BOARD_COM -b $BOARD_BAUD -D -v -C "$ARDUINO_HOME/hardware/tools/avr/etc/avrdude.conf" -U flash:w:bin/$PROJECT.hex:i
    }
    default {
        puts "Unknown OS"
    }
}
