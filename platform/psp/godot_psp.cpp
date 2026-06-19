/**************************************************************************/
/*  godot_psp.cpp                                                         */
/**************************************************************************/

#include "os_psp.h"

#include "core/profiling/profiling.h"
#include "main/main.h"

#include <GL/glut.h>
#include <pspkernel.h>
#include <psppower.h>
#include <pspthreadman.h>

#include <cstdlib>

PSP_MODULE_INFO("Godot 4 PSP", PSP_MODULE_USER, 0, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(-1);

static int exit_callback(int, int, void *) {
	sceKernelExitGame();
	return 0;
}

static int callback_thread(SceSize, void *) {
	const int callback_id = sceKernelCreateCallback("Godot exit callback", exit_callback, nullptr);
	sceKernelRegisterExitCallback(callback_id);
	sceKernelSleepThreadCB();
	return 0;
}

static void install_exit_callback() {
	const int thread_id = sceKernelCreateThread("Godot callback thread", callback_thread, 0x11, 0xFA0, 0, nullptr);
	if (thread_id >= 0) {
		sceKernelStartThread(thread_id, 0, nullptr);
	}
}

int main(int argc, char *argv[]) {
	install_exit_callback();
	scePowerSetClockFrequency(333, 333, 166);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA);
	glutInitWindowSize(480, 272);
	glutCreateWindow("Godot 4 PSP");

	godot_init_profiler();
	OS_PSP os;

	char *engine_args[] = {
		const_cast<char *>("--path"),
		const_cast<char *>("."),
		const_cast<char *>("--display-driver"),
		const_cast<char *>("psp"),
		const_cast<char *>("--rendering-method"),
		const_cast<char *>("dummy"),
	};
	Error err = Main::setup(argv[0], 6, engine_args);
	if (err != OK) {
		godot_cleanup_profiler();
		return EXIT_FAILURE;
	}

	if (Main::start() == EXIT_SUCCESS) {
		os.run();
	} else {
		os.set_exit_code(EXIT_FAILURE);
	}

	Main::cleanup();
	godot_cleanup_profiler();
	return os.get_exit_code();
}
