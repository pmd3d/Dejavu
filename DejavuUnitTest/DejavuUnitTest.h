#pragma once

#include <ctime>

namespace jlimdev
{
    class ReviewItemBuilder
    {
        uint correctReviewStreak = 0;
        uint reviewDate = 0;
        uint previousCorrectReview = 0;
        uint difficultyRating = DifficultyRatingEasiest;

    public:
        ReviewItemBuilder()
        {
            difficultyRating = DifficultyRatingEasiest;
        }

        ReviewItemBuilder& NeverReviewed(StudySession& session)
        {
            session.AddNeverReviewed();

            return *this;
        }

        ReviewItemBuilder& Due(StudySession& session, int count)
        {
            int now = static_cast<int>(time(nullptr));

            reviewDate = now - (10 * 24 * 60 * 60) - 1;
            previousCorrectReview = now - (12 * 24 * 60 * 60);
            difficultyRating = DifficultyRatingMostDifficult;

            for (int i = 0; i < count; i++)
            {
                session.AddPreviouslyCorrect(difficultyRating, reviewDate, previousCorrectReview);
            }

            return *this;
        }

        ReviewItemBuilder& Future(StudySession& session, int count)
        {
            int now = static_cast<int>(time(nullptr));
            reviewDate = now - (2 * 24 * 60 * 60);
            previousCorrectReview = now - (12 * 24 * 60 * 60);
            difficultyRating = DifficultyRatingEasiest;

            for (int i = 0; i < count; i++)
            {
                session.AddPreviouslyCorrect(difficultyRating, reviewDate, previousCorrectReview);
            }

            return *this;
        }

        ReviewItemBuilder& WithCorrectReviewStreak(int correctReviewStreak)
        {
            correctReviewStreak = correctReviewStreak;

            return *this;
        }

        ReviewItemBuilder& WithLastReviewDate(uint lastReviewDate)
        {
            reviewDate = lastReviewDate;

            return *this;
        }

        ReviewItemBuilder& WitPreviousReviewDate(int daysBeforeToReviewDate)
        {
            previousCorrectReview = reviewDate + (daysBeforeToReviewDate * -1 * 24 * 60 * 60);

            return *this;
        }

        ReviewItemBuilder& WithDifficultyRating(int difficultyPercentage)
        {
            difficultyRating = difficultyPercentage;

            return *this;
        }

        ReviewItemBuilder& WithDifficultyRating(DifficultyRating difficulty)
        {
            difficultyRating = difficulty;

            return *this;
        }
    };





    class ReviewItemListBuilder
    {
    public:
        ReviewItemListBuilder& WithNewItems(StudySession& session, int count)
        {
            for (int i = 0; i < count; i++)
            {
                ReviewItemBuilder().NeverReviewed(session);
            }

            return *this;
        }

        ReviewItemListBuilder& WithExistingItems(StudySession& session, int count)
        {
            ReviewItemBuilder().Due(session, count);

            return *this;
        }

        ReviewItemListBuilder& WithFutureItems(StudySession& session, int count)
        {
            ReviewItemBuilder().Future(session, count);

            return *this;
        }

        ReviewItemListBuilder& WithDueItems(StudySession& session, int count)
        {
            ReviewItemBuilder().Due(session, count);

            return *this;
        }
    };
}