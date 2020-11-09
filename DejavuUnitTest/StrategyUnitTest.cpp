#include "CppUnitTest.h"
#include "..\Dejavu\Dejavu.h"
#include "DejavuUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace jlimdev;

namespace FlashcardUnitTest
{
    TEST_CLASS(StrategyUnitTest)
    {
        uint now;
        SuperMemo2ReviewStrategy strategy;
        std::unique_ptr<StudySession> session;

        const uint maxNewCardsInSession = 5;
        const uint maxExistingCardsInSession = 5;
        const uint maxCards = 10000;

        TEST_METHOD_INITIALIZE(setup_session_and_strategy)
        {
            // method initialization code
            now = static_cast<int>(time(nullptr));
            session = std::make_unique<StudySession>(strategy, maxNewCardsInSession, maxExistingCardsInSession, maxCards);
        }

    public:
        TEST_METHOD(one_day_interval_for_items_without_correct_review)
        {
            session->AddNeverReviewed();

            auto nextReview = session->GetNextReviewTime(0, now);

            Assert::AreEqual(nextReview, now);
        }

        TEST_METHOD(six_day_interval_after_card_is_reviewed_correctly_once)
        {
            session->AddPreviouslyFirstCorrect(DifficultyRatingEasiest, now);

            auto nextReview = session->GetNextReviewTime(0, now);

            Assert::AreEqual(nextReview, now + Days(6));
        }

        TEST_METHOD(n_plus_2_is_interval_days_since_last_review_times_easiness_factor)
        {
            session->AddPreviouslyCorrect(DifficultyRatingEasiest, now, now - Days(11));

            auto nextReview = session->GetNextReviewTime(0, now);

            Assert::AreEqual(nextReview, now + Days(25));
        }

        TEST_METHOD(short_interval_for_difficult_cards)
        {
            session->AddPreviouslyCorrect(DifficultyRatingMostDifficult, now, now - Days(11));

            auto nextReview = session->GetNextReviewTime(0, now);

            auto expectedInterval = static_cast<Timestamp>(10 * strategy.DifficultyRatingToEasinessFactor(DifficultyRatingMostDifficult));
            
            Assert::AreEqual(nextReview, now + Days(expectedInterval));
        }

        TEST_METHOD(long_interval_for_easy_cards)
        {
            session->AddPreviouslyCorrect(DifficultyRatingEasiest, now, now - Days(11));

            auto nextReview = session->GetNextReviewTime(0, now);

            auto expectedInterval = static_cast<Timestamp>(10 * strategy.DifficultyRatingToEasinessFactor(DifficultyRatingEasiest));

            Assert::AreEqual(nextReview, now + Days(expectedInterval));
        }

        TEST_METHOD(perfect_review_lowers_difficulty)
        {
            auto reviewDate = now - Days(10) - 1;
            auto previousCorrectReview = now - Days(12);
            const auto TestDifficultyRating = 50;

            session->AddPreviouslyCorrect(TestDifficultyRating, reviewDate, previousCorrectReview);

            DifficultyRating actualDifficulty = strategy.AdjustDifficulty(session->At(0), ReviewOutcome::Perfect);

            DifficultyRating expectedDifficulty = 41;

            Assert::AreEqual(actualDifficulty, expectedDifficulty);
        }

        TEST_METHOD(hesitant_review_leaves_difficulty_the_same)
        {
            auto reviewDate = now - Days(10) - 1;
            auto previousCorrectReview = now - Days(12);
            const auto TestDifficultyRating = 50;

            session->AddPreviouslyCorrect(TestDifficultyRating, reviewDate, previousCorrectReview);

            DifficultyRating actualDifficulty = strategy.AdjustDifficulty(session->At(0), ReviewOutcome::Hesitant);

            DifficultyRating expectedDifficulty = 50;

            Assert::AreEqual(actualDifficulty, expectedDifficulty);
        }

        TEST_METHOD(incorrect_review_increases_difficulty)
        {
            auto reviewDate = now - Days(10) - 1;
            auto previousCorrectReview = now - Days(12);
            const auto TestDifficultyRating = 50;

            session->AddPreviouslyCorrect(TestDifficultyRating, reviewDate, previousCorrectReview);

            DifficultyRating actualDifficulty = strategy.AdjustDifficulty(session->At(0), ReviewOutcome::Incorrect);

            DifficultyRating expectedDifficulty = 61;

            Assert::AreEqual(actualDifficulty, expectedDifficulty);
        }

        TEST_METHOD(difficulty_can_not_be_greater_than_100)
        {
            auto reviewDate = now - Days(10) - 1;
            auto previousCorrectReview = now - Days(12);
            const auto TestDifficultyRating = 100;

            session->AddPreviouslyCorrect(TestDifficultyRating, reviewDate, previousCorrectReview);

            DifficultyRating actualDifficulty = strategy.AdjustDifficulty(session->At(0), ReviewOutcome::Incorrect);

            DifficultyRating expectedDifficulty = 100;

            Assert::AreEqual(actualDifficulty, expectedDifficulty);
        }

        TEST_METHOD(difficulty_can_not_be_less_than_0)
        {
            auto reviewDate = now - Days(10) - 1;
            auto previousCorrectReview = now - Days(12);
            const auto TestDifficultyRating = 0;

            session->AddPreviouslyCorrect(TestDifficultyRating, reviewDate, previousCorrectReview);

            DifficultyRating actualDifficulty = strategy.AdjustDifficulty(session->At(0), ReviewOutcome::Perfect);

            DifficultyRating expectedDifficulty = 0;

            Assert::AreEqual(actualDifficulty, expectedDifficulty);
        }

        Timestamp Days(uint count)
        {
            return count * 24 * 60 * 60;
        }
    };
}