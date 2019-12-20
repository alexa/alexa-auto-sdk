#include <aal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#define MODULE_ID 0
#define BUFFER_SIZE 4096

#define log(msg, ...) printf("[Player] " msg "\n", ##__VA_ARGS__)

typedef struct {
	pthread_t thread;
	pthread_mutex_t lock;
	aal_handle_t handle;
	const char *file_name;
	bool streaming;
} player_ctx_t;

static void on_start_callback(void *user_data)
{
	player_ctx_t *ctx = (player_ctx_t *) user_data;
	log("Player started");
}

static void on_stop_callback(aal_status_t reason, void *user_data)
{
	player_ctx_t *ctx = (player_ctx_t *) user_data;
	switch (reason) {
	case AAL_SUCCESS:
		log("Player finished playing");
		break;
	case AAL_PAUSED:
		log("Player paused playing");
		break;
	case AAL_ERROR:
		log("Player encountered error");
		break;
	case AAL_UNKNOWN:
		log("Player stopped playing for unknown reason");
		break;
	}
}

static void *streaming_loop(void *argument)
{
	player_ctx_t *ctx = (player_ctx_t *) argument;
	FILE *fp;
	char buffer[BUFFER_SIZE];
	size_t r;

	fp = fopen(ctx->file_name, "r");
	if (!fp) {
		log("File (%s) cannot be opened, notify EOS", ctx->file_name);
		goto eos;
	}

	do {
		r = fread(buffer, sizeof (uint8_t), BUFFER_SIZE, fp);
		if (r > 0) {
			log("Write to player %ld", r);
			ssize_t w = aal_player_write(ctx->handle, buffer, r);
			if (w != r) {
				log("aal_player_write failed written=%ld vs size=%ld", w, r);
				// Will retry...
			}
		}
		if (r != BUFFER_SIZE) {
			log("File reaches EOF or error");
			goto eos;
		}
	} while (true);

eos:
	aal_player_notify_end_of_stream(ctx->handle);
	if (fp)
		fclose(fp);

	return NULL;
}

static int start_streaming_thread(player_ctx_t *ctx)
{
	pthread_attr_t attr;
	bool attr_init = false;
	int r;

	r = pthread_attr_init(&attr);
	if (r < 0) goto bail;

	attr_init = true;

	r = pthread_create(&ctx->thread, &attr, streaming_loop, ctx);
	if (r < 0) goto bail;

bail:
	if (attr_init)
		pthread_attr_destroy(&attr);

	return r;
}

static void on_data_requested_callback(void *user_data)
{
	player_ctx_t *ctx = (player_ctx_t *) user_data;
	//log("Data requested, start streaming...");
	if (ctx->streaming) {
		//log("Ignore...");
		return;
	}
	if (start_streaming_thread(ctx) == 0) {
		ctx->streaming = true;
	}
}

static const aal_listener_t listener = {
	.on_start = on_start_callback,
	.on_stop = on_stop_callback,
	.on_almost_done = NULL,
	.on_data = NULL,
	.on_data_requested = on_data_requested_callback
};

static aal_handle_t create_player(player_ctx_t *ctx, const char *uri)
{
	const aal_attributes_t attr = {
		.name = "SampleApp",
		.device = NULL,
		.uri = uri,
		.listener = &listener,
		.user_data = ctx,
		.module_id = MODULE_ID
	};

	return aal_player_create(&attr);
}

int main(int argc, char **argv) {
	player_ctx_t ctx = {0};
	int c;

	if (argc < 2) {
		log("Usage: player <uri>|stream [<file_to_stream>]");
		return EXIT_FAILURE;
	}

	if (!aal_initialize(MODULE_ID)) {
		log("aal_initialize failed");
		return EXIT_FAILURE;
	}

	if (strcmp(argv[1], "stream") == 0) {
		/* Create Player in stream mode */
		ctx.handle = create_player(&ctx, NULL);
		if (ctx.handle) {
			log("Setup LPCM streaming mode with file: %s", argv[2]);
			aal_player_set_stream_type(ctx.handle, AAL_STREAM_LPCM);
			ctx.file_name = argv[2];
		}
	} else {
		ctx.handle = create_player(&ctx, argv[1]);
	}
	if (!ctx.handle) {
		log("aal_player_create failed");
		goto deinit;
	}

	log("Play...");
	aal_player_play(ctx.handle);

	do {
		c = getc(stdin);
		switch (c) {
		case 'p':
			aal_player_pause(ctx.handle);
			break;
		case 's':
			aal_player_stop(ctx.handle);
			break;
		case 'q':
			log("Shutdown...");
			goto deinit;
		}
	} while (c != EOF);

deinit:
	if (ctx.handle)
		aal_player_destroy(ctx.handle);

	aal_deinitialize(MODULE_ID);
	return EXIT_SUCCESS;
}