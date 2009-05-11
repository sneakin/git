#ifndef TRANSPORT_H
#define TRANSPORT_H

#include "cache.h"
#include "remote.h"

struct transport {
	struct remote *remote;
	const char *url;
	void *data;
	const struct ref *remote_refs;

	/**
	 * Returns 0 if successful, positive if the option is not
	 * recognized or is inapplicable, and negative if the option
	 * is applicable but the value is invalid.
	 **/
	int (*set_option)(struct transport *connection, const char *name,
			  const char *value);

	/**
	 * Returns a list of the remote side's refs. In order to allow
	 * the transport to try to share connections, for_push is a
	 * hint as to whether the ultimate operation is a push or a fetch.
	 *
	 * If the transport is able to determine the remote hash for
	 * the ref without a huge amount of effort, it should store it
	 * in the ref's old_sha1 field; otherwise it should be all 0.
	 **/
	struct ref *(*get_refs_list)(struct transport *transport, int for_push);

	/**
	 * Fetch the objects for the given refs. Note that this gets
	 * an array, and should ignore the list structure.
	 *
	 * If the transport did not get hashes for refs in
	 * get_refs_list(), it should set the old_sha1 fields in the
	 * provided refs now.
	 **/
	int (*fetch)(struct transport *transport, int refs_nr, struct ref **refs);

	/**
	 * Push the objects and refs. Send the necessary objects, and
	 * then tell the remote side to update each ref in the list
	 * from old_sha1 to new_sha1.
	 *
	 * Where possible, set the status for each ref appropriately.
	 *
	 * If, in the process, the transport determines that the
	 * remote side actually responded to the push by updating the
	 * ref to a different value, the transport should modify the
	 * new_sha1 in the ref. (Note that this is a matter of the
	 * remote accepting but rewriting the change, not rejecting it
	 * and reporting that a different update had already taken
	 * place)
	 **/
	int (*push_refs)(struct transport *transport, struct ref *refs, int flags);
	int (*push)(struct transport *connection, int refspec_nr, const char **refspec, int flags);

	/** get_refs_list(), fetch(), and push_refs() can keep
	 * resources (such as a connection) reserved for futher
	 * use. disconnect() releases these resources.
	 **/
	int (*disconnect)(struct transport *connection);
	char *pack_lockfile;
	signed verbose : 2;
	/* Force progress even if the output is not a tty */
	unsigned progress : 1;
};

#define TRANSPORT_PUSH_ALL 1
#define TRANSPORT_PUSH_FORCE 2
#define TRANSPORT_PUSH_DRY_RUN 4
#define TRANSPORT_PUSH_MIRROR 8
#define TRANSPORT_PUSH_VERBOSE 16

/* Returns a transport suitable for the url */
struct transport *transport_get(struct remote *, const char *);

/* Transport options which apply to git:// and scp-style URLs */

/* The program to use on the remote side to send a pack */
#define TRANS_OPT_UPLOADPACK "uploadpack"

/* The program to use on the remote side to receive a pack */
#define TRANS_OPT_RECEIVEPACK "receivepack"

/* Transfer the data as a thin pack if not null */
#define TRANS_OPT_THIN "thin"

/* Keep the pack that was transferred if not null */
#define TRANS_OPT_KEEP "keep"

/* Limit the depth of the fetch if not null */
#define TRANS_OPT_DEPTH "depth"

/* Aggressively fetch annotated tags if possible */
#define TRANS_OPT_FOLLOWTAGS "followtags"

/**
 * Returns 0 if the option was used, non-zero otherwise. Prints a
 * message to stderr if the option is not used.
 **/
int transport_set_option(struct transport *transport, const char *name,
			 const char *value);

int transport_push(struct transport *connection,
		   int refspec_nr, const char **refspec, int flags);

const struct ref *transport_get_remote_refs(struct transport *transport);

int transport_fetch_refs(struct transport *transport, struct ref *refs);
void transport_unlock_pack(struct transport *transport);
int transport_disconnect(struct transport *transport);
char *transport_anonymize_url(const char *url);

#endif
