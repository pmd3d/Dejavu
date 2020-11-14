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
    uint i;

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

void ConvertDifficultyRatingAndReviewDate(int difficultyRatingJs, i64 reviewDateJs,
    DifficultyRating& difficultyRating, Timestamp& reviewDate)
{    
    if (difficultyRatingJs < 0 ||
        difficultyRatingJs > 100)
        exit(2);

    if (reviewDateJs < 0)
        exit(3);

    reviewDate = static_cast<Timestamp>(reviewDateJs);
    difficultyRating = static_cast<uint>(difficultyRatingJs);
}

uint ConvertNow(i64 nowJs)
{
    if (nowJs < 0 || nowJs > UINT32_MAX)
        exit(1);

    return static_cast<uint>(nowJs);
}

uint ConvertIndex(int iJs)
{
    if (iJs < 0 || iJs > maxCards)
        exit(1);

    return static_cast<uint>(iJs);
}


extern "C" {

    void AddNeverReviewedCard()
    {
        session().AddNeverReviewed();
    }

    void AddPreviouslyIncorrect(int difficultyRatingJs, i64 reviewDateJs)
    {
        Timestamp reviewDate;
        DifficultyRating difficultyRating;

        ConvertDifficultyRatingAndReviewDate(difficultyRatingJs, reviewDateJs,
            difficultyRating, reviewDate);

        session().AddPreviouslyIncorrect(difficultyRating, reviewDate);
    }

    void AddPreviouslyFirstCorrect(int difficultyRatingJs, i64 reviewDateJs)
    {
        Timestamp reviewDate;
        uint difficultyRating;

        ConvertDifficultyRatingAndReviewDate(difficultyRatingJs, reviewDateJs,
            difficultyRating, reviewDate);

        session().AddPreviouslyFirstCorrect(difficultyRating, reviewDate);
    }

    void AddPreviouslyCorrect(int difficultyRatingJs, i64 reviewDateJs, i64 previousCorrectReviewJs)
    {
        if (previousCorrectReviewJs < 0)
            exit(1);

        Timestamp reviewDate;
        uint difficultyRating;

        ConvertDifficultyRatingAndReviewDate(difficultyRatingJs, reviewDateJs,
            difficultyRating, reviewDate);

        Timestamp previousCorrectReview = static_cast<Timestamp>(previousCorrectReviewJs);

        session().AddPreviouslyCorrect(difficultyRating, reviewDate, previousCorrectReview);
    }

    void SetOutcome(int iJs, int userState, i64 nowJs)
    {
        uint i = ConvertIndex(iJs);
        uint now = ConvertNow(nowJs);

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

    int Next(i64 nowJs)
    {
        uint now = ConvertNow(nowJs);

        return session().NextReview(now).value_or(-1);
    }

    void Download(int iJs)
    {
        uint i = ConvertIndex(iJs);

        std::visit(DownloadHelper{ i }, session().At(i));
    }

    void GetNextTime(int iJs, i64 nowJs)
    {
        uint i = ConvertIndex(iJs);
        uint now = ConvertNow(nowJs);

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