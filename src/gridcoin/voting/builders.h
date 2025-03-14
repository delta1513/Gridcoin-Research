// Copyright (c) 2014-2021 The Gridcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or https://opensource.org/licenses/mit-license.php.

#ifndef GRIDCOIN_VOTING_BUILDERS_H
#define GRIDCOIN_VOTING_BUILDERS_H

#include "gridcoin/voting/poll.h"

#include <memory>
#include <vector>

class CWallet;
class CWalletTx;
class uint256;

namespace GRC {
//!
//! \brief Constructs poll contract objects from user input.
//!
//! This is a move-only type. It provides a fluent interface that applies
//! validation for each of the poll component builder methods.
//!
class PollBuilder
{
public:
    //!
    //! \brief Initialize a new poll builder.
    //!
    PollBuilder();

    //!
    //! \brief Move constructor.
    //!
    PollBuilder(PollBuilder&& builder);

    //!
    //! \brief Destructor.
    //!
    ~PollBuilder();

    //!
    //! \brief Move assignment operator.
    //!
    PollBuilder& operator=(PollBuilder&& builder);

    //!
    //! \brief SetPayloadVersion
    //!
    //! \throws VotingError If the version is not valid for the current wallet height.
    //!
    PollBuilder SetPayloadVersion(uint32_t version);

    //!
    //! \brief Set the type of the poll.
    //!
    //! \throws VotingError If the supplied poll type is not valid.
    //!
    PollBuilder SetType(const PollType type);

    //!
    //! \brief Set the type of poll from the integer representation of the type.
    //!
    //! \throws VotingError If the supplied poll type is not valid.
    //!
    PollBuilder SetType(const int64_t type);

    //!
    //! \brief Set the vote weighing method for the poll.
    //!
    //! \throws VotingError If the supplied weight type is not valid.
    //!
    PollBuilder SetWeightType(const PollWeightType type);

    //!
    //! \brief Set the vote weighing method for the poll from the integer
    //! representation of the weight type.
    //!
    //! \throws VotingError If the supplied weight type is not valid.
    //!
    PollBuilder SetWeightType(const int64_t type);

    //!
    //! \brief Set the method for choosing poll answers.
    //!
    //! \throws VotingError If the supplied response type is not valid.
    //!
    PollBuilder SetResponseType(const PollResponseType type);

    //!
    //! \brief Set the method for choosing poll answers from the integer
    //! representation of the response type.
    //!
    //! \throws VotingError If the supplied response type is not valid.
    //!
    PollBuilder SetResponseType(const int64_t type);

    //!
    //! \brief Set the number of days that the poll remains active.
    //!
    //! \throws VotingError If the supplied duration is outside of the range
    //! of days for a valid poll.
    //!
    PollBuilder SetDuration(const uint32_t days);

    //!
    //! \brief Set the title of the poll.
    //!
    //! \param title A non-empty UTF-8 encoded string.
    //!
    //! \throws VotingError If the supplied title is empty or longer than the
    //! allowed size for a poll title.
    //!
    PollBuilder SetTitle(std::string title);

    //!
    //! \brief Set the URL of the poll discussion webpage.
    //!
    //! \param url A non-empty UTF-8 encoded string.
    //!
    //! \throws VotingError If the supplied URL is empty or longer than the
    //! allowed size for a poll URL.
    //!
    PollBuilder SetUrl(std::string url);

    //!
    //! \brief Set the prompt that voters shall answer.
    //!
    //! \param question A non-empty UTF-8 encoded string.
    //!
    //! \throws VotingError If the supplied question is empty or longer than
    //! the allowed size for a poll question.
    //!
    PollBuilder SetQuestion(std::string question);

    //!
    //! \brief Set the set of possible answer choices for the poll.
    //!
    //! \param labels A set of non-empty UTF-8 encoded choice display labels.
    //!
    //! \throws VotingError If a label is empty or longer than the allowed size
    //! for a poll choice, or if the set of choices exceeds the maximum allowed
    //! number for a poll, or if the set of choices contains a duplicate label.
    //!
    PollBuilder SetChoices(std::vector<std::string> labels);

    //!
    //! \brief Add a set of possible answer choices for the poll.
    //!
    //! \param labels A set of non-empty UTF-8 encoded choice display labels.
    //!
    //! \throws VotingError If a label is empty or longer than the allowed size
    //! for a poll choice, or if the set of choices exceeds the maximum allowed
    //! number for a poll, or if the set of choices contains a duplicate label.
    //!
    PollBuilder AddChoices(std::vector<std::string> labels);

    //!
    //! \brief Add a possible answer choice for the poll.
    //!
    //! \param label A non-empty UTF-8 encoded string.
    //!
    //! \throws VotingError If a label is empty or longer than the allowed size
    //! for a poll choice, or if the set of choices exceeds the maximum allowed
    //! number for a poll, or if the set of choices contains a duplicate label.
    //!
    PollBuilder AddChoice(std::string label);

    //!
    //! \brief Set the set of additional fields for the poll. SetType() should be called beforehand.
    //!
    //! \param field A set of AdditionalFields to set.
    //!
    //! \throws VotingError If any of the fields are malformed, if the set of fields
    //! contains a duplicate name, or the required boolean(s) are improperly set.
    //!
    PollBuilder SetAdditionalFields(std::vector<Poll::AdditionalField> fields);

    //!
    //! \brief Set the set of additional fields for the poll. SetType() should be called beforehand.
    //!
    //! \param fields A set of AdditionalFields to set.
    //!
    //! \throws VotingError If any of the fields are malformed, or if the set of fields
    //! contains a duplicate name, or the required boolean(s) are improperly set.
    //!
    PollBuilder SetAdditionalFields(Poll::AdditionalFieldList fields);

    //!
    //! \brief Add a set of additional fields for the poll. SetType() should be called beforehand.
    //!
    //! \param fields A set of AdditionalFields to add.
    //!
    //! \throws VotingError If any of the fields are malformed, or if the set of fields
    //! contains a duplicate name, or the required boolean(s) are improperly set.
    //!
    PollBuilder AddAdditionalFields(std::vector<Poll::AdditionalField> fields);

    //!
    //! \brief Add a set of additional fields for the poll. SetType() should be called beforehand.
    //!
    //! \param fields A set of AdditionalFields to add.
    //!
    //! \throws VotingError If any of the fields are malformed, or if the set of fields
    //! contains a duplicate name, or the required boolean(s) are improperly set.
    //!
    PollBuilder AddAdditionalFields(Poll::AdditionalFieldList fields);

    //!
    //! \brief Add an additional field for the poll.
    //!
    //! \param field The additional field name-value to add.
    //!
    //! \throws VotingError If the field is malformed, or if the set of fields
    //! contains a duplicate name.
    //!
    PollBuilder AddAdditionalField(Poll::AdditionalField field);

    //!
    //! \brief Generate a poll contract transaction with the constructed poll.
    //!
    //! \param pwallet Points to a wallet instance to generate the claim from.
    //!
    //! \return A new transaction that contains the poll contract.
    //!
    //! \throws VotingError If the constructed poll is malformed.
    //!
    CWalletTx BuildContractTx(CWallet* const pwallet);

private:
    std::unique_ptr<Poll> m_poll;    //!< The poll under construction.
    uint32_t m_poll_payload_version; //!< The poll payload version appropriate for the current block height
}; // PollBuilder

//!
//! \brief Constructs vote contract objects from user input.
//!
//! This is a move-only type. It provides a fluent interface that applies
//! validation for each of the vote component builder methods.
//!
class VoteBuilder
{
public:
    //!
    //! \brief Move constructor.
    //!
    VoteBuilder(VoteBuilder&& builder);

    //!
    //! \brief Destructor.
    //!
    ~VoteBuilder();

    //!
    //! \brief Initialize a vote builder for the specified poll.
    //!
    //! \param poll      Poll to cast a vote for.
    //! \param poll_txid Transaction hash of the associated poll.
    //!
    static VoteBuilder ForPoll(const Poll& poll, const uint256 poll_txid);

    //!
    //! \brief Move assignment operator.
    //!
    VoteBuilder& operator=(VoteBuilder&& builder);

    //!
    //! \brief Set the vote responses from poll choice offsets.
    //!
    //! \param offsets The offsets of the poll choices to vote for.
    //!
    //! \throws VotingError If an offset exceeds the bounds of the choices in
    //! the poll, or if the set of responses contains a duplicate choice.
    //!
    VoteBuilder SetResponses(const std::vector<uint8_t>& offsets);

    //!
    //! \brief Set the vote responses from poll choice labels.
    //!
    //! \param labels The UTF-8 encoded labels of the poll choices to vote for.
    //!
    //! \throws VotingError If a label does not match the label of a choice in
    //! the poll, or if the set of responses contains a duplicate choice.
    //!
    //! \deprecated Referencing voting items by strings is deprecated. Instead,
    //! use poll choice offsets to select responses for a vote.
    //!
    VoteBuilder SetResponses(const std::vector<std::string>& labels);

    //!
    //! \brief Add vote responses from poll choice offsets.
    //!
    //! \param offsets The offsets of the poll choices to vote for.
    //!
    //! \throws VotingError If an offset exceeds the bounds of the choices in
    //! the poll, or if the set of responses contains a duplicate choice.
    //!
    VoteBuilder AddResponses(const std::vector<uint8_t>& offsets);

    //!
    //! \brief Add vote responses from poll choice labels.
    //!
    //! \param labels The UTF-8 encoded labels of the poll choices to vote for.
    //!
    //! \throws VotingError If a label does not match the label of a choice in
    //! the poll, or if the set of responses contains a duplicate choice.
    //!
    //! \deprecated Referencing voting items by strings is deprecated. Instead,
    //! use poll choice offsets to select responses for a vote.
    //!
    VoteBuilder AddResponses(const std::vector<std::string>& labels);

    //!
    //! \brief Add a vote response from a poll choice offset.
    //!
    //! \param offset The offset of the poll choice to vote for.
    //!
    //! \throws VotingError If an offset exceeds the bounds of the choices in
    //! the poll, or if the set of responses contains a duplicate choice.
    //!
    VoteBuilder AddResponse(const uint8_t offset);

    //!
    //! \brief Add a vote response from a poll choice label.
    //!
    //! \param label The UTF-8 encoded label of the poll choice to vote for.
    //!
    //! \throws VotingError If a label does not match the label of a choice in
    //! the poll, or if the set of responses contains a duplicate choice.
    //!
    //! \deprecated Referencing voting items by strings is deprecated. Instead,
    //! use poll choice offsets to select responses for a vote.
    //!
    VoteBuilder AddResponse(const std::string& label);

    //!
    //! \brief Generate a vote contract transaction with the constructed vote.
    //!
    //! \param pwallet Points to a wallet instance to generate the claim from.
    //!
    //! \return A new transaction that contains the vote contract.
    //!
    //! \throws VotingError If the constructed vote is malformed.
    //!
    CWalletTx BuildContractTx(CWallet* const pwallet);

private:
    const Poll* m_poll;           //!< The poll to create a vote contract for.
    std::unique_ptr<Vote> m_vote; //!< The vote under construction.

    //!
    //! \brief Initialize a vote builder for the specified poll.
    //!
    //! \param poll      Poll to cast a vote for.
    //! \param poll_txid Transaction hash of the associated poll.
    //!
    VoteBuilder(const Poll& poll, const uint256 poll_txid);
}; // VoteBuilder

//!
//! \brief Send a transaction that contains a poll contract.
//!
//! This helper abstracts the transaction-sending code from the GUI layer. We
//! may want to replace it with a sub-routine in a view model when we rewrite
//! the voting GUI components.
//!
//! \param builder An initialized poll builder instance to create the poll
//! contract from.
//!
//! \return The hash of the transaction that contains the new poll.
//!
//! \throws VotingError If the constructed vote is malformed or the transaction
//! fails to send.
//!
uint256 SendPollContract(PollBuilder builder);

//!
//! \brief Send a transaction that contains a vote contract.
//!
//! This helper abstracts the transaction-sending code from the GUI layer. We
//! may want to replace it with a sub-routine in a view model when we rewrite
//! the voting GUI components.
//!
//! \param builder An initialized vote builder instance to create the vote
//! contract from.
//!
//! \return The hash of the transaction that contains the vote.
//!
//! \throws VotingError If the constructed vote is malformed or the transaction
//! fails to send.
//!
uint256 SendVoteContract(VoteBuilder builder);
}

#endif // GRIDCOIN_VOTING_BUILDERS_H
