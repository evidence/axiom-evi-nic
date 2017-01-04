#ifndef AXIOM_NIC_API_USER_h
#define AXIOM_NIC_API_USER_h
/*!
 * \file axiom_nic_api_user.h
 *
 * \version     v0.9
 * \date        2016-03-14
 *
 * This file contains the AXIOM NIC API for the userspace
 *
 */

#include "axiom_nic_types.h"
#include <sys/uio.h>

/*! \brief unique application ID */
#define AXIOM_ENV_ALLOC_APPID           "AXIOM_ALLOC_APPID"

/*!
 * \brief  axiom arguments for the axiom_open() function
 */
struct axiom_args {
    axiom_flags_t       flags;  /*!< \brief Axiom flags */
};

/*!
 * \brief This function open an axiom device
 *
 * \param args          The axiom open arguments
 *
 * \return Returns a pointer to the axiom device private data pointer,
 *         NULL otherwise.
 */
axiom_dev_t *
axiom_open(axiom_args_t *args);

/*!
 * \brief This function close an axiom device
 *
 * \param dev           The axiom device private data pointer
 */
void
axiom_close(axiom_dev_t *dev);

/*!
 * \brief This function set the flags of the axiom device.
 *
 * This function can be used to change the behaviour of the API. The available
 * flags are:
 *   - AXIOM_FLAGS_NOBLOCK
 *   - AXIOM_FLAGS_NOFLUSH
 *
 * \param dev           The axiom device private data pointer
 * \param flags         Axiom flags (multiple flags can be passed)
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
axiom_err_t
axiom_set_flags(axiom_dev_t *dev, axiom_flags_t flags);

/*!
 * \brief This function unset the flags of the axiom device.
 *
 * \param dev           The axiom device private data pointer
 * \param flags         Axiom flags
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
axiom_err_t
axiom_unset_flags(axiom_dev_t *dev, axiom_flags_t flags);

/*!
 * \brief This function returns the file descriptors of the axiom device, that
 *        can be used with select/poll/epoll.
 *
 * \param dev           The axiom device private data pointer
 * \param raw_fd        The fd to handle RAW messages
 * \param lonf_fd       The fd to handle LONG messages
 * \param rdma_fd        The fd to handle RDMA messages
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
axiom_err_t
axiom_get_fds(axiom_dev_t *dev, int *raw_fd, int *long_fd, int *rdma_fd);

/*!
 * \brief This function bind the current process on a specified port
 *
 * \param dev           The axiom device private data pointer
 * \param port          Port number (specify AXIOM_PORT_ANY to bind a random
 *                      port available)
 *
 * \return Returns the port bound on success, an error (< 0) otherwise.
 */
axiom_err_t
axiom_bind(axiom_dev_t *dev, axiom_port_t port);

/*!
 * \brief  This function return the interface to reach a specified node
 *
 * \param dev           The axiom device private data pointer
 * \param dst_id        Node id of target node
 * \param if_number     Interface id to reache a node
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
axiom_err_t
axiom_next_hop(axiom_dev_t *dev, axiom_node_id_t dst_id,
        axiom_if_id_t *if_number);

/*!
 * \brief  This function sends data to a remote node. If the payload_size fits
 *         the RAW payload, it is used, otherwise a LONG message is used.
 *
 * \param dev           The axiom device private data pointer
 * \param dst_id        The remote node id that will receive the raw data or
 *                      local interface that will send the raw data
 * \param port          port of the raw message
 * \param payload_size  size of data to be sent
 * \param payload       data to be sent
 *
 * \return Returns a unique positive message id on success, an error otherwise.
 */
axiom_err_t
axiom_send(axiom_dev_t *dev, axiom_node_id_t dst_id, axiom_port_t port,
        size_t payload_size, void *payload);

/*!
 * \brief This function receives RAW or LONG data from a remote node.
 *
 * \param dev           The axiom device private data pointer
 * \param src_id        The source node id that sent the long data or local
 *                      interface that received the long data
 * \param port          port of the long message
 * \param type          type of the raw message
 * \param payload_size  size of data received
 * \param payload       data received
 *
 * \return Returns a unique positive message id on success, an error otherwise.
 */
axiom_err_t
axiom_recv(axiom_dev_t *dev, axiom_node_id_t *src_id, axiom_port_t *port,
        axiom_type_t *type, size_t *payload_size, void *payload);

/*!
 * \brief  This function sends data to a remote node using iovec.
 *         If the payload_size fits the RAW payload, it is used, otherwise a
 *         LONG message is used.
 *
 * \param dev           The axiom device private data pointer
 * \param dst_id        The remote node id that will receive the raw data or
 *                      local interface that will send the raw data
 * \param port          port of the raw message
 * \param payload_size  size of data to be sent
 * \param payload       data to be sent
 *
 * \return Returns a unique positive message id on success, an error otherwise.
 */
axiom_err_t
axiom_send_iov(axiom_dev_t *dev, axiom_node_id_t dst_id, axiom_port_t port,
        size_t payload_size, struct iovec *iov, int iovcnt);

/*!
 * \brief This function receives RAW or LONG data from a remote node
 *        using iovec.
 *
 * \param dev           The axiom device private data pointer
 * \param src_id        The source node id that sent the long data or local
 *                      interface that received the long data
 * \param port          port of the long message
 * \param type          type of the raw message
 * \param payload_size  size of data received
 * \param payload       data received
 *
 * \return Returns a unique positive message id on success, an error otherwise.
 */
axiom_err_t
axiom_recv_iov(axiom_dev_t *dev, axiom_node_id_t *src_id, axiom_port_t *port,
        axiom_type_t *type, size_t *payload_size, struct iovec *iov,
        int iovcnt);

/*!
 * \brief  This function sends raw data to a remote node.
 *
 * \param dev           The axiom device private data pointer
 * \param dst_id        The remote node id that will receive the raw data or
 *                      local interface that will send the raw data
 * \param port          port of the raw message
 * \param type          type of the raw message
 * \param payload_size  size of data to be sent
 * \param payload       data to be sent
 *
 * \return Returns a unique positive message id on success, an error otherwise.
 */
axiom_err_t
axiom_send_raw(axiom_dev_t *dev, axiom_node_id_t dst_id, axiom_port_t port,
        axiom_type_t type, axiom_raw_payload_size_t payload_size,
        void *payload);

/*!
 * \brief This function receives raw data from a remote node.
 *
 * \param dev           The axiom device private data pointer
 * \param src_id        The source node id that sent the raw data or local
 *                      interface that received the raw data
 * \param port          port of the raw message
 * \param type          type of the raw message
 * \param payload_size  size of data received
 * \param payload       data received
 *
 * \return Returns a unique positive message id on success, an error otherwise.
 */
axiom_err_t
axiom_recv_raw(axiom_dev_t *dev, axiom_node_id_t *src_id, axiom_port_t *port,
        axiom_type_t *type, axiom_raw_payload_size_t *payload_size,
        void *payload);

/*!
 * \brief  This function sends raw data to a remote node using iovec.
 *
 * \param dev           The axiom device private data pointer
 * \param dst_id        The remote node id that will receive the raw data or
 *                      local interface that will send the raw data
 * \param port          port of the raw message
 * \param type          type of the raw message
 * \param payload_size  size of data to be sent
 * \param iov           iov that contains the data to be sent
 * \param iovcnt        number of iov
 *
 * \return Returns a unique positive message id on success, an error otherwise.
 */
axiom_err_t
axiom_send_iov_raw(axiom_dev_t *dev, axiom_node_id_t dst_id, axiom_port_t port,
        axiom_type_t type, axiom_raw_payload_size_t payload_size,
        struct iovec *iov, int iovcnt);

/*!
 * \brief This function receives raw data from a remote node using iovec.
 *
 * \param dev           The axiom device private data pointer
 * \param src_id        The source node id that sent the raw data or local
 *                      interface that received the raw data
 * \param port          port of the raw message
 * \param type          type of the raw message
 * \param payload_size  size of data received
 * \param iov           iov that contains the data received
 * \param iovcnt        number of iov
 *
 * \return Returns a unique positive message id on success, an error otherwise.
 */
axiom_err_t
axiom_recv_iov_raw(axiom_dev_t *dev, axiom_node_id_t *src_id, axiom_port_t *port,
        axiom_type_t *type, axiom_raw_payload_size_t *payload_size,
        struct iovec *iov, int iovcnt);

/*!
 * \brief This function returns the number of slot available to send raw
 *        messages.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns the number of slot available to send raw messages.
 * XXX: for now return 0 when there are not slot available, 1 otherwise. (TBC)
 */
int
axiom_send_raw_avail(axiom_dev_t *dev);

/*!
 * \brief This function flushes all raw messages enqueued on port previously
 *        bound.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
axiom_err_t
axiom_flush_raw(axiom_dev_t *dev);

/*!
 * \brief This function returns the number of raw messages to receive available.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns the number of raw messages to receive available.
 * XXX: for now return 0 when there are not slot available, 1 otherwise. (TBC)
 */
int
axiom_recv_raw_avail(axiom_dev_t *dev);

/*!
 * \brief  This function sends long data to a remote node.
 *
 * \param dev           The axiom device private data pointer
 * \param dst_id        The remote node id that will receive the long data or
 *                      local interface that will send the long data
 * \param port          port of the long message
 * \param payload_size  size of data to be sent
 * \param payload       data to be sent
 *
 * \return Returns a unique positive message id on success, an error otherwise.
 */
axiom_err_t
axiom_send_long(axiom_dev_t *dev, axiom_node_id_t dst_id, axiom_port_t port,
        axiom_long_payload_size_t payload_size, void *payload);

/*!
 * \brief This function receives long data from a remote node.
 *
 * \param dev           The axiom device private data pointer
 * \param src_id        The source node id that sent the long data or local
 *                      interface that received the long data
 * \param port          port of the long message
 * \param payload_size  size of data received
 * \param payload       data received
 *
 * \return Returns a unique positive message id on success, an error otherwise.
 */
axiom_err_t
axiom_recv_long(axiom_dev_t *dev, axiom_node_id_t *src_id, axiom_port_t *port,
        axiom_long_payload_size_t *payload_size, void *payload);

/*!
 * \brief  This function sends long data to a remote node using iovec.
 *
 * \param dev           The axiom device private data pointer
 * \param dst_id        The remote node id that will receive the long data or
 *                      local interface that will send the long data
 * \param port          port of the long message
 * \param payload_size  size of data to be sent
 * \param iov           iov that contains the data to be sent
 * \param iovcnt        number of iov
 *
 * \return Returns a unique positive message id on success, an error otherwise.
 */
axiom_err_t
axiom_send_iov_long(axiom_dev_t *dev, axiom_node_id_t dst_id, axiom_port_t port,
        axiom_long_payload_size_t payload_size, struct iovec *iov, int iovcnt);

/*!
 * \brief This function receives long data from a remote node using iovec.
 *
 * \param dev           The axiom device private data pointer
 * \param src_id        The source node id that sent the long data or local
 *                      interface that received the long data
 * \param port          port of the long message
 * \param payload_size  size of data received
 * \param iov           iov that contains the data received
 * \param iovcnt        number of iov
 *
 * \return Returns a unique positive message id on success, an error otherwise.
 */
axiom_err_t
axiom_recv_iov_long(axiom_dev_t *dev, axiom_node_id_t *src_id, axiom_port_t *port,
        axiom_long_payload_size_t *payload_size, struct iovec *iov, int iovcnt);

/*!
 * \brief This function returns the number of slot available to send long
 *        messages.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns the number of slot available to send long messages.
 * XXX: for now return 0 when there are not slot available, 1 otherwise. (TBC)
 */
int
axiom_send_long_avail(axiom_dev_t *dev);

/*!
 * \brief This function flushes all long messages enqueued on port previously
 *        bound.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
axiom_err_t
axiom_flush_long(axiom_dev_t *dev);

/*!
 * \brief This function returns the number of long messages to receive available.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns the number of long messages to receive available.
 * XXX: for now return 0 when there are not slot available, 1 otherwise. (TBC)
 */
int
axiom_recv_long_avail(axiom_dev_t *dev);

/*!
 * \brief This function writes data to a remote node memory.
 *
 * \param dev             The axiom device private data pointer
 * \param remote_id       The remote node id where data will be stored
 * \param payload_size    size of data to be transfer (must be multiple of 8)
 * \param local_src_addr  local address inside the RDMA zone where data
 *                        will be read
 * \param remote_dst_addr remote address inside the RDMA zone where data
 *                        will be stored
 * \param token           token that can be used to check the status of the RDMA
 *                        (it can be NULL)
 *
 * \return Returns a unique positive message id on success, an error otherwise.
 */
axiom_err_t
axiom_rdma_write(axiom_dev_t *dev, axiom_node_id_t remote_id,
        size_t payload_size, void *local_src_addr, void *remote_dst_addr,
        axiom_token_t *token);

/*!
 * \brief This function reads data from a remote node memory.
 *
 * \param dev             The axiom device private data pointer
 * \param remote_id       The remote node id where data will be read
 * \param payload_size    size of data to be transfer (must be multiple of 8)
 * \param remote_src_addr remote address inside the RDMA zone where data
 *                        will be read
 * \param local_dst_addr  local address inside the RDMA zone where data
 *                        will be stored
 * \param token           token that can be used to check the status of the RDMA
 *                        (it can be NULL)
 *
 * \return Returns a unique positive message id on success, an error otherwise.
 */
axiom_err_t
axiom_rdma_read(axiom_dev_t *dev, axiom_node_id_t remote_id,
        size_t payload_size, void *remote_src_addr, void *local_dst_addr,
        axiom_token_t *token);

/*!
 * \brief This function checks if the RDMA is completed.
 *
 * \param dev             the axiom device private data pointer
 * \param token           token that can be used to check the status of the RDMA
 *
 * \return returns a unique positive message id on success,
 *         AXIOM_RET_NOTAVAIL if the rdma is not completed or a generic error.
 */
axiom_err_t
axiom_rdma_check(axiom_dev_t *dev, axiom_token_t *token);

/*!
 * \brief This function waits a completion of the RDMA.
 *
 * \param dev             the axiom device private data pointer
 * \param token           token that can be used to check the status of the RDMA
 *
 * \return returns a unique positive message id on success or a generic error.
 */
axiom_err_t
axiom_rdma_wait(axiom_dev_t *dev, axiom_token_t *token);

/*!
 * \brief This function map in the userspace process the RDMA zone
 *
 * \param dev             The axiom device private data pointer
 * \param size            return the size of the rdma zone
 *
 * \return Returns a pointer to the RDMA zone on success, NULL otherwise.
 *
 * \deprecated This function can be used to map all RDMA region. (To use only
 *             for debug)
 */
void *
axiom_rdma_mmap(axiom_dev_t *dev, size_t *size);


/*!
 * \brief This function unmap from the userspace process the RDMA zone
 *
 * \param dev             The axiom device private data pointer
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 *
 * \deprecated This function can be used to unmap all RDMA region.
 */
axiom_err_t
axiom_rdma_munmap(axiom_dev_t *dev);

/*!
 * \brief This function reads the NI status register.
 *
 * \param               dev The axiom device private data pointer
 *
 * \return Returns the status register value.
 */
uint32_t
axiom_read_ni_status(axiom_dev_t *dev);

/*!
 * \brief This function reads the HW counter value associated with a specific
 *        RDMA request id.
 *
 * \param dev           The axiom device private data pointer
 * \param msg_id        The RDMA request id that is pending data.
 *
 * \return Returns the HW counter value associated with a specific RDMA request
 *         id.
 */
uint32_t
axiom_read_hw_counter(axiom_dev_t *dev, axiom_msg_id_t msg_id);

/*!
 * \brief This function sets the control registers for enabling local
 *        transmission ACKs and/or the PHY loopback mode configuration.
 *
 * \param dev           The axiom device private data pointer
 * \param reg_mask The register mask to be used for the configuration.
 */
void
axiom_set_ni_control(axiom_dev_t *dev, uint32_t reg_mask);

/*!
 * \brief This function reads the control register.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns the control register value.
 */
uint32_t
axiom_read_ni_control(axiom_dev_t *dev);

/*!
 * \brief This function sets the id of a local node.
 *
 * \param dev           The axiom device private data pointer
 * \param node_id       The Id assigned to this node.
 */
void
axiom_set_node_id(axiom_dev_t *dev, axiom_node_id_t node_id);

/*!
 * \brief This function returns the local node id.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns the local node id.
 */
axiom_node_id_t
axiom_get_node_id(axiom_dev_t *dev);

#if 0
/*!
 * \brief This function sets the routing table of a particular node.
 *
 * \param dev           The axiom device private data pointer
 * \param dst_node      Remote node id to setup its routing table
 * \param nodeRoutingTable  Routing table to be sent
 */
void setRoutingTable(axiom_node_id_t dst_node, uint32_t* nodeRoutingTable);
/*XXX: NOT NECESSARY */
#endif

/*!
 * \brief This function sets the routing table of a local node.
 *
 * \param dev           The axiom device private data pointer
 * \param node_id       Remote connected node id
 * \param enabled_mask  bit mask interface
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
axiom_err_t
axiom_set_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t enabled_mask);

#if 0
/*!
 * \brief This function returns the routing table of a node.
 *
 * \param dev           The axiom device private data pointer
 * \param dst_node      Remote node id to read its routing table
 *
 * \return Returns the routing table of a node.
 */
uint32_t* getRoutingTable(axiomnode_id_t dst_node);
/*XXX: NOT NECESSARY */
#endif

/*!
 * \brief This function gets the routing table of a local node.
 *
 * \param dev           The axiom device private data pointer
 * \param node_id       Remote connected node id
 * \param enabled_mask  bit mask interface
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
axiom_err_t
axiom_get_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t *enabled_mask);

/*!
 * \brief This function returns the number of nodes in the network, including
 *        the local node.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns the number of nodes on success, -1 otherwise.
 */
int
axiom_get_num_nodes(axiom_dev_t *dev);

#if 0
/*!
 * \brief This function updates the routing table of a node for a particular
 *        entry.
 *
 * \param dst_node       Remote node id to update its routing table
 * \param nodeIdToUpdate Entry to update within the routing table
 */
void updateRoutingTable(axiom_node_id_t dst_node, axiom_node_id_t nodeIdToUpdate);
/*XXX: NOT NECESSARY */

/*!
 * \brief This function invalidates an entry of the dst_node routing table.
 *
 * \param dst_node       Remote node id to remove entry from its routing table
 * \param nodeIdToUpdate Node id that no longer belongs to the routing table of
 *        dst_node
 */
void deleteNodeFromRoutingTable(axiom_node_id_t dst_node,
                                axiom_node_id_t nodeIdToRemove);
/*XXX: NOT NECESSARY */

/*!
 * \brief This function sends an identification packet to the neighbor node
 *        connected to an IF, and receives its id.
 *
 * \param dev           The axiom device private data pointer
 * \param if_id         Online interface Id where the identification message will be sent
 *
 * \return Returns the id of the neighbor node .
 */
axiom_node_id_t identifyNeighborNode(axiom_dev_t *dev, axiom_if_id_t if_id);
/*XXX: NOT NECESSARY */

/*!
 * \brief This function transmits the node’s neighbor – interface pairs to the
 *        "master node".
 *
 * \param dev           The axiom device private data pointer
 */
void reportNeighbors(axiom_dev_t *dev);
/*XXX: NOT NECESSARY */
#endif

/*!
 * \brief This function reads the number of interfaces which are present on a
 *        node.
 *
 * \param dev           The axiom device private data pointer
 * \param if_number     The number of the node interfaces
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
axiom_err_t
axiom_get_if_number(axiom_dev_t *dev, axiom_if_id_t *if_number);

/*!
 * \brief This function reads the features of an interface.
 *
 * \param dev           The axiom device private data pointer
 * \param if_number     The number of the node interface
 * \param if_features   The features of the node interface
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
axiom_err_t
axiom_get_if_info(axiom_dev_t *dev, axiom_if_id_t if_number,
        uint8_t *if_features);


axiom_err_t
axiom_debug_info(axiom_dev_t *dev);


#endif /* !AXIOM_NIC_API_USER_h */
