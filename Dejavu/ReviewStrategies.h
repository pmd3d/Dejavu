#pragma once

namespace jlimdev
{
    enum class ReviewOutcome
    {
        Perfect,
        Hesitant,
        Incorrect,
        NeverReviewed
    };

    class IReviewStrategy
    {
    public:
        virtual ~IReviewStrategy() { };
        virtual Timestamp NextReview(const ReviewItem& item, const Timestamp& now) const = 0;
        virtual DifficultyRating AdjustDifficulty(const ReviewItem& item, const ReviewOutcome& outcome) const = 0;
    };

    class SimpleReviewStrategy : public IReviewStrategy
    {
    public:
        // -- rule of 5, since derived class made destructor virtual for future..
        SimpleReviewStrategy() noexcept {}
        SimpleReviewStrategy(const SimpleReviewStrategy& s) = default;
        SimpleReviewStrategy(SimpleReviewStrategy&& s) = default;
        SimpleReviewStrategy& operator=(const SimpleReviewStrategy& s) = default;
        SimpleReviewStrategy& operator=(SimpleReviewStrategy&& s) = default;
        ~SimpleReviewStrategy() = default;

        Timestamp NextReview(const ReviewItem& item, const Timestamp& now) const noexcept override
        {
            return now;
        }

        DifficultyRating AdjustDifficulty(const ReviewItem& item, const ReviewOutcome& outcome) const noexcept override
        {
            return DifficultyRatingEasiest;
        }
    };

    /// <summary>
    /// Implementation of the SuperMemo2 algorithm described here: http://www.supermemo.com/english/ol/sm2.htm
    /// </summary>
    class SuperMemo2ReviewStrategy : public IReviewStrategy
    {
    public:
        // -- rule of 5, since derived class remember to make destructor virtual..
        SuperMemo2ReviewStrategy() noexcept {}
        SuperMemo2ReviewStrategy(const SuperMemo2ReviewStrategy& s) = default;
        SuperMemo2ReviewStrategy(SuperMemo2ReviewStrategy&& s) = default;
        SuperMemo2ReviewStrategy& operator=(const SuperMemo2ReviewStrategy& s) = default;
        SuperMemo2ReviewStrategy& operator=(SuperMemo2ReviewStrategy&& s) = default;
        ~SuperMemo2ReviewStrategy() = default;

        Timestamp NextReview(const ReviewItem& item, const Timestamp& now) const noexcept override;

        DifficultyRating AdjustDifficulty(const ReviewItem& item, const ReviewOutcome& reviewOutcome) const noexcept override;

        static double DifficultyRatingToEasinessFactor(uint difficultyRating) noexcept;

    private:
        static double ConvertOutcomeToNumber(const ReviewOutcome& reviewOutcome) noexcept;

        static double EasinessFactorToDifficultyRating(double easinessFactor) noexcept;
    };
}