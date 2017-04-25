#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <liblightnvm_cli.h>

static int KEEP_RUNNING = 1;
static int DO_ERASE = 0;

void sighandler(int signum)
{
	switch(signum) {
	case SIGUSR1:
		DO_ERASE = 1;
		break;

	case SIGUSR2:
		KEEP_RUNNING = 0;
		break;
	default:
		break;
	}
}

ssize_t _vblk_erase(struct nvm_cli *cli, struct nvm_vblk *vblk)
{
	ssize_t res = 0;

	nvm_vblk_pr(vblk);

	nvm_cli_timer_start();
	res = nvm_vblk_erase(vblk);
	if (res < 0)
		nvm_cli_perror("nvm_vblk_erase");
	nvm_cli_timer_stop();
	nvm_cli_timer_pr("nvm_vblk_erase");

	return res;
}

int line_erase(struct nvm_cli *cli)
{
	struct nvm_addr bgn = cli->args.addrs[0],
			end = cli->args.addrs[1];
	struct nvm_vblk *vblk = NULL;
	ssize_t res = 0;
	
	vblk = nvm_vblk_alloc_line(cli->args.dev, bgn.g.ch, end.g.ch, bgn.g.lun,
				   end.g.lun, bgn.g.blk);
	if (!vblk) {
		nvm_cli_perror("nvm_vblk_alloc");
		return -1;
	}

	signal(SIGUSR1, sighandler);
	signal(SIGUSR2, sighandler);

	while (KEEP_RUNNING) {
		if (DO_ERASE) {
			res = _vblk_erase(cli, vblk);
			DO_ERASE = 0;
		} else {
			sleep(0.1);
		}
	}
	
	nvm_vblk_free(vblk);

	return res < 0 ? -1 : 0;
}

//
// Remaining code is CLI boiler-plate
//
static struct nvm_cli_cmd cmds[] = {
	{"line_erase",	line_erase,	NVM_CLI_ARG_VBLK_LINE,	NVM_CLI_OPT_DEFAULT},
};

/* Define the CLI */
static struct nvm_cli cli = {
	.title = "NVM Virtual Block Eraser (nvm_vblk_*)",
	.descr_short = "Send USR1 for erase send USR2 for exit",
	.cmds = cmds,
	.ncmds = sizeof(cmds) / sizeof(cmds[0]),
};

/* Initialize and run */
int main(int argc, char **argv)
{
	int res = 0;

	if (nvm_cli_init(&cli, argc, argv) < 0) {
		nvm_cli_perror("FAILED");
		return 1;
	}

	res = nvm_cli_run(&cli);

	nvm_cli_destroy(&cli);

	return res;
}
