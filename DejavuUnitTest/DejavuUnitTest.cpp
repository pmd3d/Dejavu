#include "CppUnitTest.h"
#include "..\Dejavu\Dejavu.h"
#include "DejavuUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace jlimdev;

namespace FlashcardUnitTest
{
	TEST_CLASS(FlashcardUnitTest)
	{
        const ReviewOutcome incorrect = ReviewOutcome::Incorrect;
        const ReviewOutcome hesitant = ReviewOutcome::Hesitant;
        const ReviewOutcome perfect = ReviewOutcome::Perfect;

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
		TEST_METHOD(incorrect_items_should_be_reviewed_again)
		{
            ReviewItemListBuilder().WithNewItems(*session, 2);

            uint index = session->NextReview(now).value();

            // get it wrong...
            session->UpdateCard(index, incorrect, now);

            // next should be first
            Assert::AreEqual(index, 0U);

            // next should be second...

            index = session->NextReview(now).value();

            // get it wrong...
            session->UpdateCard(index, incorrect, now);

            Assert::AreEqual(index, 1U);

            // we should circle back to first and second because both were wrong...
            index = session->NextReview(now).value();

            // get it wrong...
            session->UpdateCard(index, incorrect, now);

            // next should be first
            Assert::AreEqual(index, 0U);

            index = session->NextReview(now).value();

            // next should be second
            Assert::AreEqual(index, 1U);
        }

        TEST_METHOD(perfect_items_should_be_removed_from_review)
        {
            ReviewItemListBuilder().WithNewItems(*session, 2);

            uint index = session->NextReview(now).value();

            // next should be first
            Assert::AreEqual(index, 0U);

            // get it wrong...
            session->UpdateCard(index, incorrect, now);

            // next should be second...

            index = session->NextReview(now).value();

            Assert::AreEqual(index, 1U);

            // get it wrong...
            session->UpdateCard(index, incorrect, now);

            // we should circle back to first and second because both were wrong...
            index = session->NextReview(now).value();

            // next should be first
            Assert::AreEqual(index, 0U);

            // get it right...
            session->UpdateCard(index, perfect, now);

            index = session->NextReview(now).value();

            // next should be first
            Assert::AreEqual(index, 1U);

            // get it right...
            session->UpdateCard(index, perfect, now);

            // we are done
            Assert::AreEqual(session->NextReview(now).has_value(), false);
        }

        TEST_METHOD(hesistate_items_should_be_removed_from_review)
        {
            ReviewItemListBuilder().WithNewItems(*session, 3);

            int index = session->NextReview(now).value();

            // next should be first
            Assert::AreEqual(index, 0);

            session->UpdateCard(index, hesitant, now);

            // next should be second...

            index = session->NextReview(now).value();

            Assert::AreEqual(index, 1);

            session->UpdateCard(index, hesitant, now);

            index = session->NextReview(now).value();

            Assert::AreEqual(index, 2);

            session->UpdateCard(index, hesitant, now);

            // we are done...
            Assert::AreEqual(session->NextReview(now).has_value(), false);
        }

        TEST_METHOD(incorrect_review_should_reset_correct_streak)
        {
            ReviewItemBuilder().Due(*session, 1);

            session->UpdateCard(0, ReviewOutcome::Incorrect, now);

            Assert::IsTrue(std::get_if<PreviouslyIncorrect>(&session->At(0)));
        }

        uint ReviewDateHelperCorrect(ReviewOutcome outcome, uint now)
        {
            ReviewItemBuilder().Due(*session, 1);

            session->UpdateCard(0, outcome, now);
            
            return std::get<PreviouslyCorrect>(session->At(0)).reviewDate;
        }

        uint ReviewDateHelperIncorrect(uint now)
        {
            ReviewItemBuilder().Due(*session, 1);

            session->UpdateCard(0, incorrect, now);

            return std::get<PreviouslyIncorrect>(session->At(0)).reviewDate;
        }

        TEST_METHOD(user_outcome_of_item_should_log_reviewdate_as_now)
        {
            Assert::AreEqual(ReviewDateHelperCorrect(perfect, now), now);
            Assert::AreEqual(ReviewDateHelperCorrect(hesitant, now), now);
            Assert::AreEqual(ReviewDateHelperIncorrect(now), now);
        }

        TEST_METHOD(stepping_through_items_should_only_cover_due_ones)
        {
            ReviewItemListBuilder()
                .WithDueItems(*session, 2)
                .WithFutureItems(*session, 3);

            int index = session->NextReview(now).value();
            Assert::AreEqual(index, 0);
            session->UpdateCard(index, hesitant, now);

            index = session->NextReview(now).value();
            Assert::AreEqual(index, 1);
            session->UpdateCard(index, hesitant, now);

            Assert::AreEqual(session->NextReview(now).has_value(), false);
        }

        TEST_METHOD(number_of_new_cards_in_one_session_should_be_limited)
        {
            ReviewItemListBuilder().WithNewItems(*session, maxNewCardsInSession + 1);
            
            uint index = 0;

            for (uint i = 0; i < maxNewCardsInSession; i++)
            {
                index = session->NextReview(now).value();
                session->UpdateCard(index, hesitant, now);
                Assert::AreEqual(index, i);
            }

            Assert::AreEqual(session->NextReview(now).has_value(), false);
        }

        TEST_METHOD(number_of_existing_cards_in_one_session_should_be_limited)
        {
            ReviewItemListBuilder().WithNewItems(*session, maxExistingCardsInSession + 1);

            uint index = 0;

            for (uint i = 0; i < static_cast<int>(maxExistingCardsInSession); i++)
            {
                index = session->NextReview(now).value();
                session->UpdateCard(index, hesitant, now);
                Assert::AreEqual(index, i);
            }

            Assert::AreEqual(session->NextReview(now).has_value(), false);
        }

        TEST_METHOD(number_of_new_cards_in_a_session_with_existing_cards_should_be_limited)
        {
            ReviewItemListBuilder().WithNewItems(*session, maxNewCardsInSession - 1)
                    .WithExistingItems(*session, 2)
                    .WithNewItems(*session, 2);

            uint index = 0;
            uint i = 0;

            for (; i < static_cast<int>(maxNewCardsInSession - 1); i++)
            {
                index = session->NextReview(now).value();
                session->UpdateCard(index, hesitant, now);
                Assert::AreEqual(index, i);
            }

            for (; i < static_cast<int>(maxNewCardsInSession + 1); i++)
            {
                index = session->NextReview(now).value();
                session->UpdateCard(index, hesitant, now);
                Assert::AreEqual(index, i);
            }

            // last new card...
            index = session->NextReview(now).value();
            session->UpdateCard(index, hesitant, now);
            Assert::AreEqual(index, i);

            // reach max of new cards...leave some in session...
            Assert::AreEqual(session->NextReview(now).has_value(), false);
        }

        TEST_METHOD(number_of_existing_cards_in_a_session_with_new_cards_should_be_limited)
        {
            ReviewItemListBuilder().WithNewItems(*session, maxNewCardsInSession - 1)
                .WithExistingItems(*session, maxExistingCardsInSession - 1)
                .WithNewItems(*session, 2)
                .WithExistingItems(*session, 2)
                .WithNewItems(*session, 2);

            uint index = 0;
            uint i = 0;

            for (; i < static_cast<int>(maxNewCardsInSession - 1); i++)
            {
                index = session->NextReview(now).value();
                session->UpdateCard(index, hesitant, now);
                Assert::AreEqual(index, i);
            }

            uint end = i + maxExistingCardsInSession - 1;

            for (; i < end; i++)
            {
                index = session->NextReview(now).value();
                session->UpdateCard(index, hesitant, now);
                Assert::AreEqual(index, i);
            }

            // last new card...
            index = session->NextReview(now).value();
            session->UpdateCard(index, hesitant, now);
            Assert::AreEqual(index, i);
            i++;
            
            // skip a new card...
            i++;

            // last existing card...leave some in session...
            index = session->NextReview(now).value();
            Assert::AreEqual(index, i);
            i++;

            Assert::AreEqual(session->NextReview(now).has_value(), false);
        }

        TEST_METHOD(future_item_should_not_be_due)
        {
            ReviewItemListBuilder().WithFutureItems(*session, 5);

            Assert::AreEqual(session->NextReview(now).has_value(), false);
        }

        TEST_METHOD(difficult_card_should_be_due_in_short_period)
        {
            Timestamp reviewDate = now - Days(1);
            auto correctReviewStreak = 2;
            Timestamp previousCorrectReview = reviewDate - Days(11);
            
            SuperMemo2ReviewStrategy strategy;

            session->AddPreviouslyCorrect(DifficultyRatingMostDifficult, reviewDate, previousCorrectReview);

            ReviewItem item = session->At(0);

            Timestamp expectedDate = now + (int)(Days(1) *
                (10 * SuperMemo2ReviewStrategy::DifficultyRatingToEasinessFactor(DifficultyRatingMostDifficult)));

            auto givenDate = strategy.NextReview(item, now);
            
            auto interval = (givenDate - now)/ (Days(1));

            Assert::AreEqual(givenDate, expectedDate);
            Assert::AreEqual(interval, 13U);
            
            // test with an outcome now...
            // review date is now, previous review 11 days ago, before that is not used
            auto reviewPriorToLastReview = 0;

            session->AddPreviouslyCorrect(DifficultyRatingMostDifficult, previousCorrectReview, reviewPriorToLastReview);

            uint afterOutcome = session->UpdateCard(1, ReviewOutcome::Perfect, now);

            auto outcomeInterval = (afterOutcome - now) / Days(1);

            Assert::AreEqual(outcomeInterval, 15U);

            // a positive outcome should make the interval longer...
            Assert::IsTrue(outcomeInterval > interval);
        }

        Timestamp Days(uint count)
        {
            return count * 24 * 60 * 60;
        }

        TEST_METHOD(js_bridge_add_card_should_return)
        {
            AddPreviouslyCorrect(91, 1601655647, 1601095676);

            Assert::IsTrue(true);
        }
    };
}
