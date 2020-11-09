// vocab tool
// supermemo 2 algorithm
// ported from https://github.com/helephant/SpacedRepetition.Net

#include "Dejavu.h"
#include <memory>

#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

using namespace jlimdev;

constexpr uint maxNewCardsInSession = 15;
constexpr uint maxExistingCardsInSession = 15;
constexpr uint maxCards = 10000;

StudySession& session() noexcept {
    // workaround for keeping global initialization in sequence with strategy constructed before session...
    static SuperMemo2ReviewStrategy strategy;
    static StudySession session(strategy, maxNewCardsInSession, maxExistingCardsInSession, maxCards);

    return session;
}

// -- bridges back and forth to javascript...

struct DownloadHelper
{
    int i;

    void operator()(NeverReviewed nr)
    {
#if __EMSCRIPTEN__
        EM_ASM(
            cppAddNeverReviewed($0)
            ,
            i
        );
#endif
    }
    void operator()(PreviouslyIncorrect p)
    {
        auto param0 = p.difficultyRating;
        auto param1 = p.reviewDate;

#if __EMSCRIPTEN__
        EM_ASM(
            cppAddPreviouslyIncorrect($0, $1, $2)
            ,
            i,
            param0,
            param1
            );
#endif
    }
    void operator()(PreviouslyFirstCorrect p)
    {
        auto param0 = p.difficultyRating;
        auto param1 = p.reviewDate;

#if __EMSCRIPTEN__
        EM_ASM(
            cppAddPreviouslyFirstCorrect($0, $1, $2)
            ,
            i,
            param0,
            param1
            );
#endif
    }
    void operator()(PreviouslyCorrect p)
    {
        auto param0 = p.difficultyRating;
        auto param1 = p.reviewDate;
        auto param2 = p.previousCorrectReview;

#if __EMSCRIPTEN__
        EM_ASM(
            cppAddPreviouslyCorrect($0, $1, $2, $3)
            ,
            i,
            param0,
            param1,
            param2
            );
#endif
    }
};

extern "C" {

    void AddNeverReviewedCard()
    {
        session().AddNeverReviewed();
    }

    void AddPreviouslyIncorrect(int difficultyRating, int reviewDate)
    {
        if (difficultyRating < 0 || reviewDate < 0)
            exit(1);

        session().AddPreviouslyIncorrect(difficultyRating, reviewDate);
    }

    void AddPreviouslyFirstCorrect(int difficultyRating, int reviewDate)
    {
        if (difficultyRating < 0 || reviewDate < 0)
            exit(1);

        session().AddPreviouslyFirstCorrect(difficultyRating, reviewDate);
    }

    void AddPreviouslyCorrect(int difficultyRating, int reviewDate, int previousCorrectReview)
    {
        if (difficultyRating < 0 || reviewDate < 0 || previousCorrectReview < 0)
            exit(1);

        session().AddPreviouslyCorrect(difficultyRating, reviewDate, previousCorrectReview);
    }

    void SetOutcome(int i, uint userState, uint now)
    {
        if (i < 0)
            exit(1);

        ReviewOutcome outcome = ReviewOutcome::Incorrect;

        switch (userState)
        {
        case 1 : 
        {
            outcome = ReviewOutcome::Incorrect;
            break;
        }
        case 2 : 
        {
            outcome = ReviewOutcome::Hesitant;
            break;
        }
        case 3 : 
        {
            outcome = ReviewOutcome::Perfect;
            break;
        }
        default :
        {
            exit(1);
        }
        }

        session().UpdateCard(i, outcome, now);
    }

    int Next(int now)
    {
        if (now < 0)
            exit(1);

        return session().NextReview(now).value_or(-1);
    }

    void Download(int i)
    {
        if (i < 0)
            exit(1);

        std::visit(DownloadHelper{ i }, session().At(i));
    }

    void GetNextTime(int i, uint now)
    {
        if (i < 0 || now < 0)
            exit(1);

        auto time = session().GetNextReviewTime(i, now);

#if __EMSCRIPTEN__
        EM_ASM(
            cppGetNextTime($0, $1)
            ,
            i,
            time
        );
#endif
    }
}

// main

int main() noexcept
{
#if __EMSCRIPTEN__
    EM_ASM(cppIsReady());
#endif

    return 0;
}