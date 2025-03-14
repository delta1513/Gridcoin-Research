// Copyright (c) 2014-2021 The Gridcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or https://opensource.org/licenses/mit-license.php.

#ifndef GRIDCOIN_VOTING_REGISTRY_H
#define GRIDCOIN_VOTING_REGISTRY_H

#include "gridcoin/contract/handler.h"
#include "gridcoin/voting/filter.h"
#include "gridcoin/voting/fwd.h"
#include "sync.h"
#include "uint256.h"
#include <atomic>
#include <map>

class CTxDB;

namespace GRC {

//!
//! \brief Thrown when a reorg/fork occurs during a poll registry traversal.
//!
class InvalidDuetoReorgFork : public std::exception
{
public:
    InvalidDuetoReorgFork()
    {
    }
};

class Contract;
class PollRegistry;

//!
//! \brief Stores an in-memory reference to a poll contract and its votes.
//!
//! The poll registry tracks recent poll and vote contracts submitted to the
//! network. The registry stores poll reference objects rather than complete
//! contracts to avoid consuming memory to maintain this state.
//!
//! This class associates votes with polls and contains the transaction hash
//! used to locate and load the poll contract from disk.
//!
class PollReference
{
    friend class PollRegistry;

public:
    //!
    //! \brief Initialize an empty, invalid poll reference object.
    //!
    PollReference();

    //!
    //! \brief Load the associated poll object from disk.
    //!
    //! \return An object that contains the associated poll if successful.
    //!
    PollOption TryReadFromDisk(CTxDB& txdb) const;

    //!
    //! \brief Load the associated poll object from disk.
    //!
    //! \return An object that contains the associated poll if successful.
    //!
    PollOption TryReadFromDisk() const;

    //!
    //! \brief Get the hash of the transaction that contains the associated poll.
    //!
    uint256 Txid() const;

    //!
    //! \brief Get the poll (payload) version
    //!
    uint32_t GetPollPayloadVersion() const;

    //!
    //! \brief Get the poll type
    //!
    PollType GetPollType() const;

    //!
    //! \brief Get the title of the associated poll.
    //!
    const std::string& Title() const;

    //!
    //! \brief Get the votes associated with the poll.
    //!
    //! \return The set of transaction hashes of the associated votes.
    //!
    const std::vector<uint256>& Votes() const;

    //!
    //! \brief Get the timestamp of the poll transaction.
    //!
    //! \return Poll transaction timestamp in seconds.
    //!
    int64_t Time() const;

    //!
    //! \brief Get the elapsed time since poll creation.
    //!
    //! \param now Timestamp to consider as the current time.
    //!
    //! \return Poll age in seconds.
    //!
    int64_t Age(const int64_t now) const;

    //!
    //! \brief Determine whether the poll age exceeds the duration of the poll.
    //!
    //! \param now Timestamp to consider as the current time.
    //!
    //! \return \c true if a poll's age exceeds the poll duration.
    //!
    bool Expired(const int64_t now) const;

    //!
    //! \brief Get the time when the poll expires.
    //!
    //! \return Expiration time as the number of seconds since the UNIX epoch.
    //!
    int64_t Expiration() const;

    //!
    //! \brief Get the block index pointer for the first block in the poll duration.
    //!
    //! \return pointer to block index object.
    //!
    CBlockIndex* GetStartingBlockIndexPtr() const;

    //!
    //! \brief Get the block index pointer for the last block in the poll duration.
    //!
    //! \return pointer to block index object.
    //!
    CBlockIndex* GetEndingBlockIndexPtr(CBlockIndex* pindex_start = nullptr) const;

    //!
    //! \brief Get the starting block height for the poll.
    //!
    //! \return block number if begun or std::nullopt (if skeleton reference - this should never happen).
    //!
    std::optional<int> GetStartingHeight() const;

    //!
    //! \brief Get the ending block height for the poll.
    //!
    //! \return block number if ended or std::nullopt if still active.
    //!
    std::optional<int> GetEndingHeight() const;

    //!
    //! \brief Computes the Active Vote Weight for the poll, which is used to determine whether the poll is validated.
    //! \param result: The actual tabulated votes (poll result)
    //! \return ActiveVoteWeight
    //!
    std::optional<CAmount> GetActiveVoteWeight(const PollResultOption &result) const;

    //!
    //! \brief Record a transaction that contains a response to the poll.
    //!
    //! \param txid Hash of the transaction that contains the vote.
    //!
    void LinkVote(const uint256 txid);

    //!
    //! \brief Remove a record of a transaction that contains a response to
    //! the poll.
    //!
    //! \param txid Hash of the transaction that contains the vote.
    //!
    void UnlinkVote(const uint256 txid);

private:
    uint256 m_txid;               //!< Hash of the poll transaction.
    uint32_t m_payload_version;   //!< Version of the poll (payload).
    PollType m_type;              //!< Type of the poll.
    const std::string* m_ptitle;  //!< Title of the poll.
    int64_t m_timestamp;          //!< Timestamp of the poll transaction.
    uint32_t m_duration_days;     //!< Number of days the poll remains active.
    std::vector<uint256> m_votes; //!< Hashes of the linked vote transactions.
}; // PollReference

//!
//! \brief Tracks recent polls and votes submitted to the network.
//!
//! THREAD SAFETY: This API uses the transaction database to read poll and vote
//! contracts from disk. Always lock \c cs_main for the poll registry, for poll
//! references, and for iterator lifetimes.
//!
class PollRegistry : public IContractHandler
{
public:
    using PollMapByTitle = std::map<std::string, PollReference>;
    using PollMapByTxid = std::map<uint256, PollReference*>;

    CCriticalSection cs_poll_registry;  //!< Lock for poll registry.

    //!
    //! \brief A traversable, immutable sequence of the polls in the registry.
    //!
    class Sequence
    {
    public:
        using FilterFlag = PollFilterFlag;

        //!
        //! \brief Behaves like a forward \c const iterator.
        //!
        class Iterator
        {
        public:
            using BaseIterator = PollMapByTitle::const_iterator;
            using difference_type = BaseIterator::difference_type;
            using iterator_category = std::forward_iterator_tag;

            using value_type = Iterator;
            using pointer = const value_type*;
            using reference = const value_type&;

            //!
            //! \brief Initialize an iterator for the beginning of the sequence.
            //!
            Iterator(
                BaseIterator iter,
                BaseIterator end,
                const FilterFlag flags,
                const int64_t now);

            //!
            //! \brief Initialize an iterator for the end of the sequence.
            //!
            Iterator(BaseIterator end);

            //!
            //! \brief Get the poll reference at the current position.
            //!
            const PollReference& Ref() const;

            //!
            //! \brief Load the poll at the current position from disk.
            //!
            PollOption TryPollFromDisk() const;

            //!
            //! \brief Get a reference to the current position.
            //!
            //! \return A reference to itself.
            //!
            reference operator*() const;

            //!
            //! \brief Get a pointer to the current position.
            //!
            //! \return A pointer to itself.
            //!
            pointer operator->() const;

            //!
            //! \brief Advance the current position.
            //!
            Iterator& operator++();

            //!
            //! \brief Advance the current position.
            //!
            Iterator operator++(int);

            //!
            //! \brief Determine whether the item at the current position is
            //! equal to the specified position.
            //!
            bool operator==(const Iterator& other) const;

            //!
            //! \brief Determine whether the item at the current position is
            //! not equal to the specified position.
            //!
            bool operator!=(const Iterator& other) const;

        private:
            BaseIterator m_iter GUARDED_BY(cs_poll_registry); //!< The current position.
            BaseIterator m_end GUARDED_BY(cs_poll_registry);  //!< Element after the end of the sequence.
            FilterFlag m_flags GUARDED_BY(cs_poll_registry);  //!< Attributes to filter polls by.
            int64_t m_now GUARDED_BY(cs_poll_registry);       //!< Current time in seconds.

            //!
            //! \brief Advance the iterator to the next item that matches the
            //! configured filters.
            //!
            void SeekNextMatch();
        }; // Iterator

        //!
        //! \brief Initialize a poll sequence.
        //!
        //! \param polls The set of poll references in the registry.
        //! \param flags Attributes to filter polls by.
        //!
        Sequence(const PollMapByTitle& polls, const FilterFlag flags = NO_FILTER);

        //!
        //! \brief Set the attributes to filter polls by.
        //!
        //! \return A new sequence for the specified poll filters.
        //!
        Sequence Where(const FilterFlag flags) const;

        //!
        //! \brief Set whether the sequence skips finished polls.
        //!
        //! \param active_only Whether the sequence skips finished polls.
        //!
        //! \return A new sequence for the specified poll status filters.
        //!
        Sequence OnlyActive(const bool active_only = true) const;

        //!
        //! \brief Returns an iterator to the beginning.
        //!
        Iterator begin() const;

        //!
        //! \brief Returns an iterator to the end.
        //!
        Iterator end() const;

    private:
        const PollMapByTitle& m_polls GUARDED_BY(cs_poll_registry) ; //!< Poll references in the registry.
        FilterFlag m_flags GUARDED_BY(cs_poll_registry) ;            //!< Attributes to filter polls by.
    }; // Sequence

    //!
    //! \brief Get a traversable sequence of the polls in the registry.
    //!
    //! \return A traversable type that iterates over the poll references in
    //! the registry and reads poll objects from disk.
    //!
    const Sequence Polls() const;

    //!
    //! \brief Get the most recent poll submitted to the network.
    //!
    //! \return Points to a poll object or \c nullptr when no recent polls
    //! exist.
    //!
    const PollReference* TryLatestActive() const;

    //!
    //! \brief Get the poll with the specified transaction ID.
    //!
    //! \param title Hash of the transaction that contains the poll to look up.
    //!
    //! \return Points to a poll object or \c nullptr when no poll exists for
    //! the supplied transaction hash.
    //!
    const PollReference* TryByTxid(const uint256 txid) const;

    //!
    //! \brief Get the poll with the specified title.
    //!
    //! \param title The title of the poll to look up.
    //!
    //! \return Points to a poll object or \c nullptr when no poll contains a
    //! matching title.
    //!
    const PollReference* TryByTitle(const std::string& title) const;

    //!
    //! \brief Get an existing poll in the registry, or if not found, demand load a
    //! poll identified by the provided txid along with its votes. This is a temporary
    //! workaround to deal with polls beyond the lookback period for contract load
    //! during wallet init. This should be replaced when the poll caching state machine
    //! code is implemented.
    //!
    //! \return Points to a poll object or \c nullptr if no poll added that matches
    //! the supplied txid.
    const PollReference* TryByTxidWithAddHistoricalPollAndVotes(const uint256 txid);

    //!
    //! \brief Destroy the contract handler state to prepare for historical
    //! contract replay.
    //!
    void Reset() override;

    //!
    //! \brief Perform contextual validation for the provided contract.
    //!
    //! \param contract Contract to validate.
    //! \param tx       Transaction that contains the contract.
    //! \param DoS      Misbehavior score out.
    //!
    //! \return \c false If the contract fails validation.
    //!
    bool Validate(const Contract& contract, const CTransaction& tx, int& DoS) const override;

    //!
    //! \brief Perform contextual validation for the provided contract including block context. This is used
    //! in ConnectBlock.
    //!
    //! \param ctx ContractContext to validate.
    //! \param DoS Misbehavior score out.
    //!
    //! \return  \c false If the contract fails validation.
    //!
    bool BlockValidate(const ContractContext& ctx, int& DoS) const override;

    //!
    //! \brief Register a poll or vote from contract data.
    //!
    //! \param ctx References the poll or vote contract and associated context.
    //!
    void Add(const ContractContext& ctx) override;

    //!
    //! \brief Deregister the poll or vote specified by contract data.
    //!
    //! \param ctx References the poll or vote contract and associated context.
    //!
    void Delete(const ContractContext& ctx) override;

    //!
    //! \brief Detect reorganizations that would affect registry traversal.
    //!
    void DetectReorg();

    std::atomic<bool> registry_traversal_in_progress = false;      //!< Boolean that registry traversal is in progress.
    std::atomic<bool> reorg_occurred_during_reg_traversal = false; //!< Boolean to indicate whether a reorg occurred.

private:
    PollMapByTitle m_polls GUARDED_BY(cs_poll_registry);             //!< Poll references keyed by title.
    PollMapByTxid m_polls_by_txid GUARDED_BY(cs_poll_registry);      //!< Poll references keyed by TXID.
    const PollReference* m_latest_poll GUARDED_BY(cs_poll_registry); //!< Cache for the most recent poll.

    //!
    //! \brief Get the poll with the specified title.
    //!
    //! \param title The title of the poll to look up.
    //!
    //! \return Points to a poll object or \c nullptr when no poll contains a
    //! matching title.
    //!
    PollReference* TryBy(const std::string& title);

    //!
    //! \brief Get the poll with the specified title.
    //!
    //! \param title The title of the poll to look up.
    //!
    //! \return Points to a poll object or \c nullptr when no poll exists for
    //! the supplied transaction hash.
    //!
    PollReference* TryBy(const uint256 txid);

    //!
    //! \brief Register a poll from contract data.
    //!
    //! \param ctx References the poll contract and associated context.
    //!
    void AddPoll(const ContractContext& ctx);

    //!
    //! \brief Register a vote from contract data.
    //!
    //! \param ctx References the vote contract and associated context.
    //!
    void AddVote(const ContractContext& ctx);

    //!
    //! \brief Deregister the poll specified by contract data.
    //!
    //! \param ctx References the poll contract and associated context.
    //!
    void DeletePoll(const ContractContext& ctx);

    //!
    //! \brief Deregister the vote specified by contract data.
    //!
    //! \param ctx References the vote contract and associated context.
    //!
    void DeleteVote(const ContractContext& ctx);
}; // PollRegistry

//!
//! \brief Get the global poll registry.
//!
//! \return Current global poll registry instance.
//!
PollRegistry& GetPollRegistry();
}

#endif // GRIDCOIN_VOTING_REGISTRY_H
