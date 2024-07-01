# Thu 20 Jun 16:19:49 UTC 2024

# all: verbose

all:	clear_semaphores terse

.PHONY: all

strange:
	@git ls-files -o

clear_semaphores:
	@rm -rf ./.semaphore*
	@> .semaphore-clear_semaphores
	@echo clear_semaphores_done
	@./scripts.d/stall.sh

verbose:	clean compile_verbose
	@sha1sum ./CURRENT.d/test-aa.ino.elf

terse:	clean compile_non_verbose
	@sha1sum ./CURRENT.d/test-aa.ino.elf

wtf_terse:	clean compile_only
	@echo COMPILE ONLY
	@sha1sum ./CURRENT.d/test-aa.ino.elf

# install:

upload:
	@date;sleep 4;date;date;date
	@arduino-cli upload --fqbn CH55xDuino:mcs51:ch552 --port /dev/ttyACM1

# @./upload.sh

compile_verbose:
	@echo COMPILE VERBOSE
	@cat ./scripts.d/compile_verbose.sh | sh

esc_char:
	@echo -n \\033

# @$(MAKE) -f $(THIS_FILE) .compile_non_verbose

compile_non_verbose:
	@$(MAKE) -s .compile_non_verbose
	@date
	@$(MAKE) -s .begin
	@echo -n  ' compiling.. '
	@cat ./scripts.d/compile_verbose.sh  | sh > /dev/null
	@$(MAKE) -s .end
	@date

compile_only:
	@echo COMPILE ONLY
	@./compile.sh

board:
	@arduino-cli board listall | ag 552

comm:
	./cf-fastpico9t-esp32-vsc.sh /dev/ttyACM0

# find all tabs and remove them in the status, to help form .gitignore entries
git_status:
	@git status | cat | tr -d '\t'

pwd:
	@echo -n '  ..'
	@pwd | cut -b 27-

clean:	.clean
	rm -rf /tmp/arduino/cores ; rm -rf /tmp/arduino/sketches
	@> .semaphore-clean

distclean:	.distclean
	@echo 'make clean' | sh > /dev/null


# ---- messages ----

.end:
	@echo -e '\033\033[0;1;34m[\033[0;33mend.\033[0m\033[0;1;34m]\033[0m'

.begin:
	@echo -e '\033[0;1;34m[\033[0;33mbegin..\033[0m\033[0;1;34m]\033[0m'

.compile_non_verbose:
	@echo -e '\033[0;1;34m[\033[0;33mcompile non-verbose\033[0m\033[0;1;34m]\033[0m'

.clean:
	@echo -e '\033[0;1;34m[\033[0;33mclean\033[0m\033[0;1;34m]\033[0m'

.distclean:
	@echo -e '\033[0;1;34m[\033[0;33mdistclean\033[0m\033[0;1;34m]\033[0m'
	@> .semaphore-distclean

# end.
