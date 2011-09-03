#include "storage.h"

#include <errno.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <string.h>
#include <sys/stat.h>

#include "logging.h"

status_t storage_create_paths(struct daemon *self, unsigned char *hash) {
    char fullpath[23];
    mode_t dirmode = 0;

    dirmode = S_IRWXU | S_IRWXG;

    strncpy(&fullpath[0], &hash[0], 2);
    fullpath[2] = 0;
    if ((mkdir(fullpath, dirmode) != 0) && (errno != EEXIST)) {
        daemon_err(self, LOG_ALERT, "Could not create path (%s): %s", fullpath, strerror(errno));
        return FAILURE;
    }

    fullpath[2] = '/';
    strncpy(&fullpath[3], &hash[2], 2);
    fullpath[5] = 0;
    if ((mkdir(fullpath, dirmode) != 0) && (errno != EEXIST)) {
        daemon_err(self, LOG_ALERT, "Could not create path (%s): %s", fullpath, strerror(errno));
        return FAILURE;
    }

    return SUCCESS;
}

char *storage_format_data_filename(struct daemon *self, unsigned char *hash) {
    char *fullpath = malloc(sizeof(char) * 23);
    strncpy(&fullpath[0], &hash[0], 2);
    fullpath[2] = '/';
    strncpy(&fullpath[3], &hash[2], 2);
    fullpath[5] = '/';
    strncpy(&fullpath[6], &hash[4], 16);
    fullpath[22] = 0;

    return fullpath;
}

status_t storage_store_stats_data(struct daemon *self, unsigned char *hash, struct stats_packet *packet) {
    char *fullpath = storage_format_data_filename(self, hash);
    FILE *fp = fopen(fullpath, "w");

    if (fp == NULL) {
        daemon_log(self, LOG_ALERT, "Could not open data file (%s): %s", fullpath, strerror(errno));
        return FAILURE;
    }

    fwrite(packet, sizeof(*packet), 1, fp);
    fclose(fp);
}

unsigned char *storage_get_stats_hash(struct daemon *self, struct stats_packet *packet) {
    int hash_len = SHA_DIGEST_LENGTH;
    unsigned char *hash = malloc(sizeof(unsigned char) * hash_len);
    EVP_MD_CTX *ctx = EVP_MD_CTX_create();
    EVP_DigestInit_ex(ctx, EVP_sha1(), NULL);

    EVP_DigestUpdate(ctx, packet->service, strlen(packet->service));
    EVP_DigestUpdate(ctx, packet->metric, strlen(packet->service));

    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_destroy(ctx);

    return hash;
}

status_t storage_store_stats(struct daemon *self, struct stats_packet *packet) {
    unsigned char *hash = NULL;
    hash = storage_get_stats_hash(self, packet);
    storage_store_stats_data(self, hash, packet);
}
