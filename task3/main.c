#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>

typedef struct {
        uint32_t uid;
	uint16_t res1;
	uint16_t res2;
        uint32_t begin_time;
        uint32_t end_time;
} session_t;

uint32_t getSeshDuration(session_t s) {
	return s.end_time - s.begin_time;
}

const int MAX_ENTRIES = 16384;

typedef struct {
	uint32_t uid;
	session_t* max_sesh;
} user_max_sesh;

int userIndexInArray(uint32_t uid, user_max_sesh* arr, int length) {
	for (int i=0; i < length; i++) {
		if (arr[i].uid == uid)
			return i;
	}
	return -1;
}

uint32_t getAvgSeshDuration(session_t* arr, int length);

uint64_t getDisperse(session_t* arr, int length);

int main(const int argc, char const* argv[]) {

	if (argc != 2)
		errx(1, "Usage: $ %s <file.bin>", argv[0]);

	int fd = open(argv[1], O_RDONLY);
	if (fd == -1)
		err(1, "could not open binary file for reading");
	
	session_t buff[MAX_ENTRIES];
	ssize_t rd_bytes = read(fd, buff, sizeof(buff));
	if (rd_bytes == -1) {
		close(fd);
		err(1, "could not load into buffer");
	}
	close(fd);

	const int entries_count = rd_bytes / sizeof(session_t);
	if (!entries_count)
		errx(1, "no entries in buffer");

	uint64_t disperse = getDisperse(buff, entries_count);

	user_max_sesh usersMaxSession[2048];
	int users_total = 0;

	for (int i = 0; i < entries_count; i++) {
		int pos = userIndexInArray(buff[i].uid, usersMaxSession, users_total);
		uint64_t dur = (uint64_t)getSeshDuration(buff[i]);
		if (dur*dur > disperse) {
			if (pos == -1) {
				user_max_sesh newEntry;
				newEntry.uid = buff[i].uid;
				newEntry.max_sesh = &buff[i];
				usersMaxSession[users_total++] = newEntry;
			} else {
				if (dur > getSeshDuration(*usersMaxSession[pos].max_sesh))
					usersMaxSession[pos].max_sesh = &buff[i];
			}
		}
	}

	for (int i=0; i < users_total; i++) {
		user_max_sesh curr = usersMaxSession[i];
		printf("%ld %ld\n", (unsigned long int)curr.uid, (unsigned long int)getSeshDuration(*curr.max_sesh));
	}

	exit(0);
}


uint32_t getAvgSeshDuration(session_t* arr, int length) {
	uint32_t avgDuration = 0;
        for (int i=0; i < length; i++) {
                avgDuration = avgDuration + getSeshDuration(arr[i]);
        }
        return avgDuration / length;
}

uint64_t getDisperse(session_t* arr, int length) {
	uint32_t avgDuration = getAvgSeshDuration(arr,length);

	uint64_t disperse = 0;
        for (int i=0; i < length; i++) {
                disperse = disperse + (uint64_t)((int64_t)getSeshDuration(arr[i]) - (int64_t)avgDuration)*((int64_t)getSeshDuration(arr[i]) - (int64_t)avgDuration);
        }
        return disperse / length;
}

