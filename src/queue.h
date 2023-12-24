#ifndef QUEUE_H
#define QUEUE_H

/**
 * @brief structure representing various fields of a data entry
 *
 * @param id unique identifier
 * @param site_name name of the site
 * @param site_url URL of the site
 * @param username username for the site
 * @param password password for the site
 * @param auth_text additional authentication text
 * @param recovery_code recovery code for the site
 * @param comment additional comments
 * @param sqltime SQL timestamp
 * @param modtime last modification time
 */
struct field
{
	char *id;
	char *site_name;
	char *site_url;
	char *username;
	char *password;
	char *auth_text;
	char *recovery_code;
	char *comment;
	char *sqltime;
	char *modtime;
};

/**
 * @brief node structure for a queue
 *
 * @param data Pointer to the field data stored in the node
 * @param next Pointer to the next node in the queue
 * @note this struct is only for internal use
 */
struct qnode
{
	struct field *data;
	struct qnode *next;
};

/**
 * @brief structure representing a queue
 *
 * @param front pointer to the front node of the queue
 * @param back pointer to the back node of the queue
 * @param size elements of the queue
 */
struct queue
{
	struct qnode *front;
	struct qnode *back;
};

/**
 * @brief creates and initializes a new queue
 * @return pointer to the newly created queue, or NULL if memory allocation fails
 */
struct queue *make_queue(void);

/**
 * @brief creates and initializes a new field structure
 * @return pointer to the newly created field structure, or NULL if memory allocation fails
 */
struct field *make_field(void);

/**
 * @brief adds a new field to the end of the queue
 * @param q pointer to the queue
 * @param data pointer to the field to be added to the queue
 * @return pointer to the added field data, or NULL if memory allocation for the new node fails
 */
struct field *enqueue(struct queue *q, struct field *data);

/**
 * @brief removes and returns the front element of the queue
 * @param q pointer to the queue
 * @return the front 'field' element of the queue, or NULL if the queue is empty
 */
struct field *dequeue(struct queue *q);

/**
 * @brief frees the memory for all fields of a 'field' struct
 * @param data pointer to the 'field' struct to be freed
 * @note no-op if 'data' is NULL, otherwise 'data' needs to be initialized
 */
void free_field(struct field *data);

/**
 * @brief frees the memory for the queue and its elements
 * @param q pointer to the queue to be freed
 * @note no-op if 'q' is NULL, otherwise 'q' needs to be initialized'
 */
void free_queue(struct queue *q);

#endif /* QUEUE_H */