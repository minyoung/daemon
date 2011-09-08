#include "storage.h"

#include <errno.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <string.h>
#include <sys/stat.h>

#include "logging.h"

status_t storage_create_paths(struct daemon *self, const char *fullpath) {
    mode_t dirmode = 0;

    dirmode = S_IRWXU | S_IRWXG;

    char subpath[6];
    strncpy(&subpath[0], &fullpath[0], 2);
    subpath[2] = 0;
    if ((mkdir(subpath, dirmode) != 0) && (errno != EEXIST)) {
        daemon_err(self, LOG_ALERT, "Could not create path (%s): %s", subpath, strerror(errno));
        return FAILURE;
    }

    subpath[2] = '/';
    strncpy(&subpath[3], &fullpath[3], 2);
    subpath[5] = 0;
    if ((mkdir(subpath, dirmode) != 0) && (errno != EEXIST)) {
        daemon_err(self, LOG_ALERT, "Could not create path (%s): %s", subpath, strerror(errno));
        return FAILURE;
    }

    return SUCCESS;
}

char *storage_format_data_filename(struct daemon *self, unsigned char *hash) {
    char *fullpath = malloc(sizeof(char) * 43);

    sprintf(&fullpath[0], "%02x/%02x/", hash[0], hash[1]);
    int i = 0;
    for (i = 2; i < 20; i++) {
        sprintf(&fullpath[2+i*2], "%02x", hash[i]);
    }
    fullpath[42] = 0;

    return fullpath;
}

status_t storage_write_header(struct daemon *self, FILE *fp, const struct stats_packet *packet) {
    if (fprintf(fp, "%s\n", STORAGE_VERSION) < 0) { goto FAIL; }
    if (fprintf(fp, "%s\n", packet->type) < 0) { goto FAIL; }
    if (fprintf(fp, "%s\n", packet->service) < 0) { goto FAIL; }
    if (fprintf(fp, "%s\n", packet->metric) < 0) { goto FAIL; }
    if (fprintf(fp, "%s\n", packet->hostname) < 0) { goto FAIL; }

    int i;
    for (i = 0; i < packet->tag_count; i++) {
        if (fprintf(fp, "%s\n", packet->tags[i]) < 0) { goto FAIL; }
    }
    if (fprintf(fp, "\n") < 0) { goto FAIL; }

    if (fwrite(&packet->timestamp, sizeof(packet->timestamp), 1, fp) != 1) { goto FAIL; }

    daemon_log(self, LOG_DEBUG, "Header written [%m]");
    return SUCCESS;

FAIL:
    daemon_err(self, LOG_ALERT, "Could not write header: %m");
    return FAILURE;
}

status_t storage_read_header(struct daemon *self, FILE *fp, struct stats_packet *header) {
    char buffer[128];

    // version
    fgets(buffer, 128, fp);

    if (fgets(buffer, 128, fp) == NULL) { goto FAIL; }
    header->type = string_copy(&header->type, buffer);

    if (fgets(buffer, 128, fp) == NULL) { goto FAIL; }
    header->service = string_copy(&header->service, buffer);

    if (fgets(buffer, 128, fp) == NULL) { goto FAIL; }
    header->metric = string_copy(&header->metric, buffer);

    if (fgets(buffer, 128, fp) == NULL) { goto FAIL; }
    header->hostname = string_copy(&header->hostname, buffer);

    int i = 0;
    for (i = 0; i < MAX_TAG_COUNT; ++i) {
        if (fgets(buffer, 128, fp) == NULL) { goto FAIL; }
        if (strlen(buffer) == 0) { break; }
        header->tags[i] = string_copy(&header->tags[i], buffer);
    }

    if (fread(&header->timestamp, sizeof(header->timestamp), 1, fp) != 1) { goto FAIL; }

    return SUCCESS;

FAIL:
    daemon_err(self, LOG_ALERT, "Error reading data file: %m");
    return FAILURE;
}

status_t storage_append_stats_file(struct daemon *self, const char *fullpath, const struct stats_packet *packet) {
    FILE *fp = fopen(fullpath, "r+");
    if (fp == NULL) {
        daemon_log(self, LOG_ALERT, "Could not open data file (%s): %m", fullpath);
        return FAILURE;
    }
    daemon_log(self, LOG_DEBUG, "Opened data file (%s) [%m]", fullpath);

    struct stats_packet header;
    storage_read_header(self, fp, &header);
    fclose(fp);

    return SUCCESS;
}

status_t storage_create_new_stats_file(struct daemon *self, const char *fullpath, const struct stats_packet *packet) {
    if (storage_create_paths(self, fullpath) != SUCCESS) {
        return FAILURE;
    }

    FILE *fp = fopen(fullpath, "w");
    if (fp == NULL) {
        daemon_err(self, LOG_ALERT, "Could not create data file (%s): %m", fullpath);
        return FAILURE;
    }
    daemon_log(self, LOG_DEBUG, "Created data file (%s) [%m]", fullpath);

    status_t ret = storage_write_header(self, fp, packet);

    fclose(fp);

    if (ret != SUCCESS) {
        return FAILURE;
    }
    return SUCCESS;
}

status_t storage_store_stats_data(struct daemon *self, unsigned char *hash, const struct stats_packet *packet) {
    char *fullpath = storage_format_data_filename(self, hash);

    status_t result = FAILURE;
    if (access(fullpath, R_OK | W_OK) == 0) {
        daemon_log(self, LOG_DEBUG, "Appending to stats file: %s [%m]", fullpath);
        result = storage_append_stats_file(self, fullpath, packet);
    } else {
        daemon_log(self, LOG_DEBUG, "Creating new stats file: %s [%m]", fullpath);
        result = storage_create_new_stats_file(self, fullpath, packet);
    }

    free(fullpath);
    return result;
}

unsigned char *storage_get_stats_hash(struct daemon *self, struct stats_packet *packet) {
    unsigned int hash_len = SHA_DIGEST_LENGTH;
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
    return storage_store_stats_data(self, hash, packet);
}
