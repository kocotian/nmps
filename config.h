#define PS1 "\033[1;97m🧡 \033[1;3%dm%d ⚡️ \033[1;3%dm%d 🥩 \033[1;3%dm%d 💀 \033[1;3%dm%d\n\033[0;37m%s@%s \033[0m$ ",\
			health > 75 ? 2 : (health > 25 ? 3 : 1), \
			health, \
			energy > 75 ? 2 : (energy > 25 ? 3 : 1), \
			energy, \
			saturation > 75 ? 2 : (saturation > 25 ? 3 : 1), \
			saturation, \
			sanity > 75 ? 2 : (sanity > 25 ? 3 : 1), \
			sanity, \
			username, host

static const char *defaultusername		= NULL;
