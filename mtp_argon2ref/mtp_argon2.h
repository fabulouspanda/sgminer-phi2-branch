/*
 * MTPArgon2 reference source code package - reference C implementations
 *
 * Copyright 2015
 * Daniel Dinu, Dmitry Khovratovich, Jean-Philippe Aumasson, and Samuel Neves
 *
 * You may use this work under the terms of a Creative Commons CC0 1.0 
 * License/Waiver or the Apache Public License 2.0, at your option. The terms of
 * these licenses can be found at:
 *
 * - CC0 1.0 Universal : http://creativecommons.org/publicdomain/zero/1.0
 * - Apache 2.0        : http://www.apache.org/licenses/LICENSE-2.0
 *
 * You should have received a copy of both of these licenses along with this
 * software. If not, they may be obtained at the above URLs.
 */

#ifndef MTP_ARGON2_H
#define MTP_ARGON2_H

#include <stdint.h>
#include <stddef.h>
#include <limits.h>

#if defined(__cplusplus)
extern "C" {
#endif

/* Symbols visibility control */
#ifdef A2_VISCTL
#define MTP_ARGON2_PUBLIC __attribute__((visibility("default")))
#elif _MSC_VER
#define MTP_ARGON2_PUBLIC __declspec(dllexport)
#else
#define MTP_ARGON2_PUBLIC
#endif

/*
 * MTPArgon2 input parameter restrictions
 */

/* Minimum and maximum number of lanes (degree of parallelism) */
#define MTP_ARGON2_MIN_LANES UINT32_C(1)
#define MTP_ARGON2_MAX_LANES UINT32_C(0xFFFFFF)

/* Minimum and maximum number of threads */
#define MTP_ARGON2_MIN_THREADS UINT32_C(1)
#define MTP_ARGON2_MAX_THREADS UINT32_C(0xFFFFFF)

/* Number of synchronization points between lanes per pass */
#define MTP_ARGON2_SYNC_POINTS UINT32_C(4)

/* Minimum and maximum digest size in bytes */
#define MTP_ARGON2_MIN_OUTLEN UINT32_C(4)
#define MTP_ARGON2_MAX_OUTLEN UINT32_C(0xFFFFFFFF)

/* Minimum and maximum number of memory argon_blocks (each of argon_block_SIZE bytes) */
#define MTP_ARGON2_MIN_MEMORY (2 * MTP_ARGON2_SYNC_POINTS) /* 2 argon_blocks per slice */

#define MTP_ARGON2_MIN(a, b) ((a) < (b) ? (a) : (b))
/* Max memory size is addressing-space/2, topping at 2^32 argon_blocks (4 TB) */
#define MTP_ARGON2_MAX_MEMORY_BITS                                                 \
    MTP_ARGON2_MIN(UINT32_C(32), (sizeof(void *) * CHAR_BIT - 10 - 1))
#define MTP_ARGON2_MAX_MEMORY                                                      \
    MTP_ARGON2_MIN(UINT32_C(0xFFFFFFFF), UINT64_C(1) << MTP_ARGON2_MAX_MEMORY_BITS)

/* Minimum and maximum number of passes */
#define MTP_ARGON2_MIN_TIME UINT32_C(1)
#define MTP_ARGON2_MAX_TIME UINT32_C(0xFFFFFFFF)

/* Minimum and maximum password length in bytes */
#define MTP_ARGON2_MIN_PWD_LENGTH UINT32_C(0)
#define MTP_ARGON2_MAX_PWD_LENGTH UINT32_C(0xFFFFFFFF)

/* Minimum and maximum associated data length in bytes */
#define MTP_ARGON2_MIN_AD_LENGTH UINT32_C(0)
#define MTP_ARGON2_MAX_AD_LENGTH UINT32_C(0xFFFFFFFF)

/* Minimum and maximum salt length in bytes */
#define MTP_ARGON2_MIN_SALT_LENGTH UINT32_C(8)
#define MTP_ARGON2_MAX_SALT_LENGTH UINT32_C(0xFFFFFFFF)

/* Minimum and maximum key length in bytes */
#define MTP_ARGON2_MIN_SECRET UINT32_C(0)
#define MTP_ARGON2_MAX_SECRET UINT32_C(0xFFFFFFFF)

/* Flags to determine which fields are securely wiped (default = no wipe). */
#define MTP_ARGON2_DEFAULT_FLAGS UINT32_C(0)
#define MTP_ARGON2_FLAG_CLEAR_PASSWORD (UINT32_C(1) << 0)
#define MTP_ARGON2_FLAG_CLEAR_SECRET (UINT32_C(1) << 1)

/* Global flag to determine if we are wiping internal memory buffers. This flag
 * is defined in core.c and deafults to 1 (wipe internal memory). */
extern int MTP_FLAG_clear_internal_memory;

/* Error codes */
typedef enum MTPArgon2_ErrorCodes {
    MTP_ARGON2_OK = 0,

    MTP_ARGON2_OUTPUT_PTR_NULL = -1,

    MTP_ARGON2_OUTPUT_TOO_SHORT = -2,
    MTP_ARGON2_OUTPUT_TOO_LONG = -3,

    MTP_ARGON2_PWD_TOO_SHORT = -4,
    MTP_ARGON2_PWD_TOO_LONG = -5,

    MTP_ARGON2_SALT_TOO_SHORT = -6,
    MTP_ARGON2_SALT_TOO_LONG = -7,

    MTP_ARGON2_AD_TOO_SHORT = -8,
    MTP_ARGON2_AD_TOO_LONG = -9,

    MTP_ARGON2_SECRET_TOO_SHORT = -10,
    MTP_ARGON2_SECRET_TOO_LONG = -11,

    MTP_ARGON2_TIME_TOO_SMALL = -12,
    MTP_ARGON2_TIME_TOO_LARGE = -13,

    MTP_ARGON2_MEMORY_TOO_LITTLE = -14,
    MTP_ARGON2_MEMORY_TOO_MUCH = -15,

    MTP_ARGON2_LANES_TOO_FEW = -16,
    MTP_ARGON2_LANES_TOO_MANY = -17,

    MTP_ARGON2_PWD_PTR_MISMATCH = -18,    /* NULL ptr with non-zero length */
    MTP_ARGON2_SALT_PTR_MISMATCH = -19,   /* NULL ptr with non-zero length */
    MTP_ARGON2_SECRET_PTR_MISMATCH = -20, /* NULL ptr with non-zero length */
    MTP_ARGON2_AD_PTR_MISMATCH = -21,     /* NULL ptr with non-zero length */

    MTP_ARGON2_MEMORY_ALLOCATION_ERROR = -22,

    MTP_ARGON2_FREE_MEMORY_CBK_NULL = -23,
    MTP_ARGON2_ALLOCATE_MEMORY_CBK_NULL = -24,

    MTP_ARGON2_INCORRECT_PARAMETER = -25,
    MTP_ARGON2_INCORRECT_TYPE = -26,

    MTP_ARGON2_OUT_PTR_MISMATCH = -27,

    MTP_ARGON2_THREADS_TOO_FEW = -28,
    MTP_ARGON2_THREADS_TOO_MANY = -29,

    MTP_ARGON2_MISSING_ARGS = -30,

    MTP_ARGON2_ENCODING_FAIL = -31,

    MTP_ARGON2_DECODING_FAIL = -32,

    MTP_ARGON2_THREAD_FAIL = -33,

    MTP_ARGON2_DECODING_LENGTH_FAIL = -34,

    MTP_ARGON2_VERIFY_MISMATCH = -35
} mtp_argon2_error_codes;

/* Memory allocator types --- for external allocation */
typedef int (*allocate_fptr)(uint8_t **memory, size_t bytes_to_allocate);
typedef void (*deallocate_fptr)(uint8_t *memory, size_t bytes_to_allocate);

/* MTPArgon2 external data structures */

/*
 *****
 * Context: structure to hold MTPArgon2 inputs:
 *  output array and its length,
 *  password and its length,
 *  salt and its length,
 *  secret and its length,
 *  associated data and its length,
 *  number of passes, amount of used memory (in KBytes, can be rounded up a bit)
 *  number of parallel threads that will be run.
 * All the parameters above affect the output hash value.
 * Additionally, two function pointers can be provided to allocate and
 * deallocate the memory (if NULL, memory will be allocated internally).
 * Also, three flags indicate whether to erase password, secret as soon as they
 * are pre-hashed (and thus not needed anymore), and the entire memory
 *****
 * Simplest situation: you have output array out[8], password is stored in
 * pwd[32], salt is stored in salt[16], you do not have keys nor associated
 * data. You need to spend 1 GB of RAM and you run 5 passes of MTPArgon2d with
 * 4 parallel lanes.
 * You want to erase the password, but you're OK with last pass not being
 * erased. You want to use the default memory allocator.
 * Then you initialize:
 MTPArgon2_Context(out,8,pwd,32,salt,16,NULL,0,NULL,0,5,1<<20,4,4,NULL,NULL,true,false,false,false)
 */
typedef struct MTPArgon2_Context {
    uint8_t *out;    /* output array */
    uint32_t outlen; /* digest length */

    uint8_t *pwd;    /* password array */
    uint32_t pwdlen; /* password length */

    uint8_t *salt;    /* salt array */
    uint32_t saltlen; /* salt length */

    uint8_t *secret;    /* key array */
    uint32_t secretlen; /* key length */

    uint8_t *ad;    /* associated data array */
    uint32_t adlen; /* associated data length */

    uint32_t t_cost;  /* number of passes */
    uint32_t m_cost;  /* amount of memory requested (KB) */
    uint32_t lanes;   /* number of lanes */
    uint32_t threads; /* maximum number of threads */

    uint32_t version; /* version number */

    allocate_fptr allocate_cbk; /* pointer to memory allocator */
    deallocate_fptr free_cbk;   /* pointer to memory deallocator */

    uint32_t flags; /* array of bool options */
} mtp_argon2_context;

/* MTPArgon2 primitive type */
typedef enum MTPArgon2_type {
  MTPArgon2_d = 0,
  MTPArgon2_i = 1,
  MTPArgon2_id = 2
} mtp_argon2_type;

/* Version of the algorithm */
typedef enum MTPArgon2_version {
    MTP_ARGON2_VERSION_10 = 0x10,
    MTP_ARGON2_VERSION_13 = 0x13,
    MTP_ARGON2_VERSION_NUMBER = MTP_ARGON2_VERSION_13
} mtp_argon2_version;
// big hack
typedef struct argon_block_ { uint64_t v[128];} argon_block;
typedef struct argon_blockS_ { uint64_t v[128];} argon_blockS;
/*
typedef struct argon_block_with_offset_ {
	argon_block memory;
	uint64_t offset;
} argon_block_with_offset;
*/
/*
typedef struct argon_block_with_offset_ {
	argon_block memory;
//	char* proof;
	char proof[4034];
} argon_block_with_offset;
*/

typedef struct MTPArgon2_instance_t {
	argon_block *memory;          /* Memory pointer */
	uint32_t version;
	uint32_t passes;        /* Number of passes */
	uint32_t memory_argon_blocks; /* Number of argon_blocks in memory */
	uint32_t segment_length;
	uint32_t lane_length;
	uint32_t lanes;
	uint32_t threads;
	mtp_argon2_type type;
	int print_internals; /* whether to print the memory argon_blocks */
	mtp_argon2_context *context_ptr; /* points back to original context */
	uint32_t argon_block_header[18]; // takes 256 first bits of the argon_blockheader //hash_0
} mtp_argon2_instance_t;


/*
 * Function that gives the string representation of an mtp_argon2_type.
 * @param type The mtp_argon2_type that we want the string for
 * @param uppercase Whether the string should have the first letter uppercase
 * @return NULL if invalid type, otherwise the string representation.
 */
MTP_ARGON2_PUBLIC const char *mtp_argon2_type2string(mtp_argon2_type type, int uppercase);

/*
 * Function that performs memory-hard hashing with certain degree of parallelism
 * @param  context  Pointer to the MTPArgon2 internal structure
 * @return Error code if smth is wrong, MTP_ARGON2_OK otherwise
 */
MTP_ARGON2_PUBLIC int mtp_argon2_ctx(mtp_argon2_context *context, mtp_argon2_type type);


MTP_ARGON2_PUBLIC int mtp_argon2_ctx_from_mtp(mtp_argon2_context *context, mtp_argon2_instance_t *instance);
/**
 * Hashes a password with MTPArgon2i, producing an encoded hash
 * @param t_cost Number of iterations
 * @param m_cost Sets memory usage to m_cost kibibytes
 * @param parallelism Number of threads and compute lanes
 * @param pwd Pointer to password
 * @param pwdlen Password size in bytes
 * @param salt Pointer to salt
 * @param saltlen Salt size in bytes
 * @param hashlen Desired length of the hash in bytes
 * @param encoded Buffer where to write the encoded hash
 * @param encodedlen Size of the buffer (thus max size of the encoded hash)
 * @pre   Different parallelism levels will give different results
 * @pre   Returns MTP_ARGON2_OK if successful
 */
MTP_ARGON2_PUBLIC int mtp_argon2i_hash_encoded(const uint32_t t_cost,
                                       const uint32_t m_cost,
                                       const uint32_t parallelism,
                                       const void *pwd, const size_t pwdlen,
                                       const void *salt, const size_t saltlen,
                                       const size_t hashlen, char *encoded,
                                       const size_t encodedlen);

/**
 * Hashes a password with MTPArgon2i, producing a raw hash by allocating memory at
 * @hash
 * @param t_cost Number of iterations
 * @param m_cost Sets memory usage to m_cost kibibytes
 * @param parallelism Number of threads and compute lanes
 * @param pwd Pointer to password
 * @param pwdlen Password size in bytes
 * @param salt Pointer to salt
 * @param saltlen Salt size in bytes
 * @param hash Buffer where to write the raw hash - updated by the function
 * @param hashlen Desired length of the hash in bytes
 * @pre   Different parallelism levels will give different results
 * @pre   Returns MTP_ARGON2_OK if successful
 */
MTP_ARGON2_PUBLIC int mtp_argon2i_hash_raw(const uint32_t t_cost, const uint32_t m_cost,
                                   const uint32_t parallelism, const void *pwd,
                                   const size_t pwdlen, const void *salt,
                                   const size_t saltlen, void *hash,
                                   const size_t hashlen);

MTP_ARGON2_PUBLIC int mtp_argon2d_hash_encoded(const uint32_t t_cost,
                                       const uint32_t m_cost,
                                       const uint32_t parallelism,
                                       const void *pwd, const size_t pwdlen,
                                       const void *salt, const size_t saltlen,
                                       const size_t hashlen, char *encoded,
                                       const size_t encodedlen);

MTP_ARGON2_PUBLIC int mtp_argon2d_hash_raw(const uint32_t t_cost, const uint32_t m_cost,
                                   const uint32_t parallelism, const void *pwd,
                                   const size_t pwdlen, const void *salt,
                                   const size_t saltlen, void *hash,
                                   const size_t hashlen);

MTP_ARGON2_PUBLIC int mtp_argon2id_hash_encoded(const uint32_t t_cost,
                                        const uint32_t m_cost,
                                        const uint32_t parallelism,
                                        const void *pwd, const size_t pwdlen,
                                        const void *salt, const size_t saltlen,
                                        const size_t hashlen, char *encoded,
                                        const size_t encodedlen);

MTP_ARGON2_PUBLIC int mtp_argon2id_hash_raw(const uint32_t t_cost,
                                    const uint32_t m_cost,
                                    const uint32_t parallelism, const void *pwd,
                                    const size_t pwdlen, const void *salt,
                                    const size_t saltlen, void *hash,
                                    const size_t hashlen);

/* generic function underlying the above ones */
MTP_ARGON2_PUBLIC int mtp_argon2_hash(const uint32_t t_cost, const uint32_t m_cost,
                              const uint32_t parallelism, const void *pwd,
                              const size_t pwdlen, const void *salt,
                              const size_t saltlen, void *hash,
                              const size_t hashlen, char *encoded,
                              const size_t encodedlen, mtp_argon2_type type,
                              const uint32_t version);

/**
 * Verifies a password against an encoded string
 * Encoded string is restricted as in validate_inputs()
 * @param encoded String encoding parameters, salt, hash
 * @param pwd Pointer to password
 * @pre   Returns MTP_ARGON2_OK if successful
 */
MTP_ARGON2_PUBLIC int mtp_argon2i_verify(const char *encoded, const void *pwd,
                                 const size_t pwdlen);

MTP_ARGON2_PUBLIC int mtp_argon2d_verify(const char *encoded, const void *pwd,
                                 const size_t pwdlen);

MTP_ARGON2_PUBLIC int mtp_argon2id_verify(const char *encoded, const void *pwd,
                                  const size_t pwdlen);

/* generic function underlying the above ones */
MTP_ARGON2_PUBLIC int mtp_argon2_verify(const char *encoded, const void *pwd,
                                const size_t pwdlen, mtp_argon2_type type);

/**
 * MTPArgon2d: Version of MTPArgon2 that picks memory argon_blocks depending
 * on the password and salt. Only for side-channel-free
 * environment!!
 *****
 * @param  context  Pointer to current MTPArgon2 context
 * @return  Zero if successful, a non zero error code otherwise
 */
MTP_ARGON2_PUBLIC int mtp_argon2d_ctx(mtp_argon2_context *context);



/**
 * MTPArgon2i: Version of MTPArgon2 that picks memory argon_blocks
 * independent on the password and salt. Good for side-channels,
 * but worse w.r.t. tradeoff attacks if only one pass is used.
 *****
 * @param  context  Pointer to current MTPArgon2 context
 * @return  Zero if successful, a non zero error code otherwise
 */
MTP_ARGON2_PUBLIC int mtp_argon2i_ctx(mtp_argon2_context *context);

/**
 * MTPArgon2id: Version of MTPArgon2 where the first half-pass over memory is
 * password-independent, the rest are password-dependent (on the password and
 * salt). OK against side channels (they reduce to 1/2-pass MTPArgon2i), and
 * better with w.r.t. tradeoff attacks (similar to MTPArgon2d).
 *****
 * @param  context  Pointer to current MTPArgon2 context
 * @return  Zero if successful, a non zero error code otherwise
 */
MTP_ARGON2_PUBLIC int mtp_argon2id_ctx(mtp_argon2_context *context);

/**
 * Verify if a given password is correct for MTPArgon2d hashing
 * @param  context  Pointer to current MTPArgon2 context
 * @param  hash  The password hash to verify. The length of the hash is
 * specified by the context outlen member
 * @return  Zero if successful, a non zero error code otherwise
 */
MTP_ARGON2_PUBLIC int mtp_argon2d_verify_ctx(mtp_argon2_context *context, const char *hash);

/**
 * Verify if a given password is correct for MTPArgon2i hashing
 * @param  context  Pointer to current MTPArgon2 context
 * @param  hash  The password hash to verify. The length of the hash is
 * specified by the context outlen member
 * @return  Zero if successful, a non zero error code otherwise
 */
MTP_ARGON2_PUBLIC int mtp_argon2i_verify_ctx(mtp_argon2_context *context, const char *hash);

/**
 * Verify if a given password is correct for MTPArgon2id hashing
 * @param  context  Pointer to current MTPArgon2 context
 * @param  hash  The password hash to verify. The length of the hash is
 * specified by the context outlen member
 * @return  Zero if successful, a non zero error code otherwise
 */
MTP_ARGON2_PUBLIC int mtp_argon2id_verify_ctx(mtp_argon2_context *context,
                                      const char *hash);

/* generic function underlying the above ones */
MTP_ARGON2_PUBLIC int mtp_argon2_verify_ctx(mtp_argon2_context *context, const char *hash,
                                    mtp_argon2_type type);

/**
 * Get the associated error message for given error code
 * @return  The error message associated with the given error code
 */
MTP_ARGON2_PUBLIC const char *mtp_argon2_error_message(int error_code);

/**
 * Returns the encoded hash length for the given input parameters
 * @param t_cost  Number of iterations
 * @param m_cost  Memory usage in kibibytes
 * @param parallelism  Number of threads; used to compute lanes
 * @param saltlen  Salt size in bytes
 * @param hashlen  Hash size in bytes
 * @param type The mtp_argon2_type that we want the encoded length for
 * @return  The encoded hash length in bytes
 */
MTP_ARGON2_PUBLIC size_t mtp_argon2_encodedlen(uint32_t t_cost, uint32_t m_cost,
                                       uint32_t parallelism, uint32_t saltlen,
                                       uint32_t hashlen, mtp_argon2_type type);

#if defined(__cplusplus)
}
#endif

#endif
