/*
  Copyright (c) 2013, Matthias Schiffer <mschiffer@universe-factory.net>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>


typedef struct job {
	struct job *next;

	uint64_t minutes;
	uint32_t hours;
	uint32_t doms;
	uint16_t months;
	uint8_t dows;

	char *command;
} job_t;


static const char const *const MONTHS[12] = {
	"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"
};

static const char const *const WEEKDAYS[7] = {
	"sun", "mon", "tue", "wed", "thu", "fri", "sat"
};


static const char *crondir;

static job_t *jobs = NULL;


static void usage(void) {
	fprintf(stderr, "Usage: micrond <crondir>\n");
}


static inline uint64_t bit(unsigned b) {
	return ((uint64_t)1) << b;
}

static int strict_atoi(const char *s) {
	char *end;
	int ret = strtol(s, &end, 10);

	if (*end)
		return -1;
	else
		return ret;
}

static uint64_t parse_strings(const char *input, const char *const *strings, size_t n) {
	size_t i;
	for (i = 0; i < n; i++) {
		if (strcasecmp(input, strings[i]) == 0)
			return bit(i);
	}

	return 0;
}

static uint64_t parse_times(char *input, int min, int n) {
	uint64_t ret = 0;
	int step = 1;

	char *comma = strchr(input, ',');
	if (comma) {
		*comma = 0;
		ret = parse_times(comma+1, min, n);

		if (!ret)
			return 0;
	}

	char *slash = strchr(input, '/');
	if (slash) {
		*slash = 0;
		step = strict_atoi(slash+1);

		if (step <= 0)
			return 0;
	}

	int begin, end;
	char *minus = strchr(input, '-');
	if (minus) {
		*minus = 0;
		begin = strict_atoi(input);
		end = strict_atoi(minus+1);
	}
	else if (strcmp(input, "*") == 0) {
		begin = min;
		end = min+n-1;
	}
	else {
		begin = end = strict_atoi(input);
	}

	if (begin < min || end < min)
		return 0;

	int i;
	for (i = begin-min; i <= end-min; i += step)
		ret |= bit(i % n);

	return ret;
}

static int handle_line(const char *line) {
	job_t job = {};
	int ret = -1;
	char *columns[5];
	int i;
	int len;

	int matches = sscanf(line, "%ms %ms %ms %ms %ms %n", &columns[0], &columns[1], &columns[2], &columns[3], &columns[4], &len);
	if (matches != 5 && matches != 6) {
		if (matches <= 0)
			ret = 0;

		goto end;
	}

	job.minutes = parse_times(columns[0], 0, 60);
	if (!job.minutes)
		goto end;

	job.hours = parse_times(columns[1], 0, 24);
	if (!job.hours)
		goto end;

	job.doms = parse_times(columns[2], 1, 31);
	if (!job.doms)
		goto end;


	job.months = parse_strings(columns[3], MONTHS, 12);

	if (!job.months)
		job.months = parse_times(columns[3], 1, 12);
	if (!job.months)
		goto end;

	job.dows = parse_strings(columns[4], WEEKDAYS, 7);
	if (!job.dows)
		job.dows = parse_times(columns[4], 0, 7);
	if (!job.dows)
		goto end;

	job.command = strdup(line+len);

	job_t *jobp = malloc(sizeof(job_t));
	*jobp = job;

	jobp->next = jobs;
	jobs = jobp;

	ret = 0;

  end:
	for (i = 0; i < matches && i < 5; i++)
		free(columns[i]);

	return ret;
}


static void read_crontab(const char *name) {
	FILE *file = fopen(name, "r");
	if (!file) {
		syslog(LOG_WARNING, "unable to read crontab `%s'", name);
		return;
	}

	char line[16384];
	unsigned lineno = 0;

	while (fgets(line, sizeof(line), file)) {
		lineno++;

		char *comment = strchr(line, '#');
		if (comment)
			*comment = 0;

		if (handle_line(line))
			syslog(LOG_WARNING, "syntax error in `%s', line %u", name, lineno);
	}

	fclose(file);
}


static void read_crondir(void) {
	DIR *dir;

	if (chdir(crondir) || ((dir = opendir(".")) == NULL)) {
		fprintf(stderr, "Unable to read crondir `%s'\n", crondir);
		usage();
		exit(1);
	}

	struct dirent *ent;
	while ((ent = readdir(dir)) != NULL) {
		if (ent->d_name[0] == '.')
			continue;

		read_crontab(ent->d_name);
	}

	closedir(dir);
}


static void run_job(const job_t *job) {
	pid_t pid = fork();
	if (pid == 0) {
		execl("/bin/sh", "/bin/sh", "-c", job->command, (char*)NULL);
		syslog(LOG_ERR, "unable to run job: exec failed");
		_exit(1);
	}
	else if (pid < 0) {
		syslog(LOG_ERR, "unable to run job: fork failed");
	}
}


static void check_job(const job_t *job, const struct tm *tm) {
	if (!(job->minutes & bit(tm->tm_min)))
		return;

	if (!(job->hours & bit(tm->tm_hour)))
		return;

	if (!(job->doms & bit(tm->tm_mday-1)))
		return;

	if (!(job->months & bit(tm->tm_mon)))
		return;

	if (!(job->dows & bit(tm->tm_wday)))
		return;

	run_job(job);
}


int main(int argc, char *argv[]) {
	if (argc != 2) {
		usage();

		exit(argc < 2 ? 0 : 1);
	}

	crondir = argv[1];

	signal(SIGCHLD, SIG_IGN);

	read_crondir();

	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	int minute = tm->tm_min;

	while (1) {
		sleep(60 - t%60);

		t = time(NULL);
		tm = localtime(&t);

		minute = (minute+1)%60;
		if (tm->tm_min != minute) {
			/* clock has moved, don't execute jobs */
			minute = tm->tm_min;
			continue;
		}

		job_t *job;
		for (job = jobs; job; job = job->next)
			check_job(job, tm);
	}
}
