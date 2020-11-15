#pragma once

#include <vector>
#include <optional>
#include <variant>

namespace jlimdev {

    enum class ReviewState
    {
        Unvisited,
        Visited,
        Wrong
    };

    class StudySession
    {
    private:
        const IReviewStrategy& _reviewStrategy;

        uint _newCardsReturned;
        uint _existingCardsReturned;
        uint _newCardMax;
        uint _existingCardMax;
        uint _cardLimit;

        std::vector<ReviewItem> _cards;
        std::vector<ReviewState> _visit;
        uint _currentIndex;

    public:
        StudySession(const IReviewStrategy& reviewStrategy, uint maxNewCard, uint maxExistingCard, uint cardLimit) noexcept;
        void AddNeverReviewed();
        void AddPreviouslyIncorrect(uint difficultyRating, Timestamp reviewDate);
        void AddPreviouslyFirstCorrect(uint difficultyRating, Timestamp reviewDate);
        void AddPreviouslyCorrect(uint difficultyRating, Timestamp reviewDate, Timestamp previousCorrectReview);
        uint UpdateCard(uint i, const ReviewOutcome& outcome, uint now);
        ReviewItem& At(uint i);
        std::optional<uint> NextReview(Timestamp now);
        void AddItem(const ReviewItem& item);
        uint GetNextReviewTime(uint i, uint now);
        void Reset();
    private:
        ReviewItem MapItem(uint i, const ReviewOutcome& outcome, Timestamp now);
        bool IsDue(const ReviewItem& item, const Timestamp& now) const;
        bool IsNewItem(const ReviewItem& item) const noexcept;
    };


}