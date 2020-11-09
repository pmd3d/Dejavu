#include "..\Dejavu\Dejavu.h"

using namespace jlimdev;

struct NextReviewSuperMemo2Visitor
{
    Timestamp now;

    Timestamp operator()(const NeverReviewed& nr)
    {
        return now;
    }
    Timestamp operator()(const PreviouslyIncorrect& p)
    {
        return now;
    }
    Timestamp operator()(const PreviouslyFirstCorrect& p)
    {
        // -- add 6 days...
        return now + 6 * 24 * 60 * 60;
    }
    Timestamp operator()(const PreviouslyCorrect& p)
    {
        const double easinessFactor = SuperMemo2ReviewStrategy::DifficultyRatingToEasinessFactor(p.difficultyRating);
        const Timestamp seconds = p.reviewDate - p.previousCorrectReview;
        const Timestamp daysSincePreviousReview = seconds / (24 * 60 * 60);
        const Timestamp daysUntilNextReview = static_cast<int>((daysSincePreviousReview - 1) * easinessFactor);

        return now + daysUntilNextReview * 24 * 60 * 60;
    }

};

Timestamp SuperMemo2ReviewStrategy::NextReview(const ReviewItem& item, const Timestamp& now) const noexcept
{
    return std::visit(NextReviewSuperMemo2Visitor{ now }, item);
}

DifficultyRating SuperMemo2ReviewStrategy::AdjustDifficulty(const ReviewItem& item, const ReviewOutcome& reviewOutcome)  const noexcept
{
    //EF':=EF+(0.1-(3-q)*(0.08+(3-q)*0.02))
    //where:
    //EF' - new value of the E-Factor,
    //EF - old value of the E-Factor,
    //q - quality of the response in the 0-3 grade scale.
    //If EF is less than 1.3 then let EF be 1.3.
    double outcome = ConvertOutcomeToNumber(reviewOutcome);
    DifficultyRating rating = std::visit(
        [](auto&& item) -> DifficultyRating { return item.difficultyRating; },
        item);
    const double currentEasinessFactor = DifficultyRatingToEasinessFactor(rating);
    const double newEasinessFactor = currentEasinessFactor + (0.1 - (3 - outcome) * (0.08 + (3 - outcome) * 0.02));
    double newDifficultyRating = EasinessFactorToDifficultyRating(newEasinessFactor);

    if (newDifficultyRating > 100)
    {
        newDifficultyRating = 100;
    }
    if (newDifficultyRating < 0)
    {
        newDifficultyRating = 0;
    }

    return static_cast<Timestamp>(newDifficultyRating);
}

double SuperMemo2ReviewStrategy::DifficultyRatingToEasinessFactor(uint difficultyRating) noexcept
{
    // using a linear equation - y = mx + b
    return (-0.012 * difficultyRating) + 2.5;
}

double SuperMemo2ReviewStrategy::ConvertOutcomeToNumber(const ReviewOutcome& reviewOutcome) noexcept
{
    double outcome = 0.0;
    if (reviewOutcome == ReviewOutcome::Perfect)
    {
        outcome = 3.0;
    }
    else if (reviewOutcome == ReviewOutcome::Hesitant)
    {
        outcome = 2.0;
    }
    else if (reviewOutcome == ReviewOutcome::Incorrect)
    {
        outcome = 1.0;
    }
    else if (reviewOutcome == ReviewOutcome::NeverReviewed)
    {
        outcome = 0.0;
    }
    return outcome;
}

double SuperMemo2ReviewStrategy::EasinessFactorToDifficultyRating(double easinessFactor) noexcept
{
    // using a linear equation - x = (y - b)/m
    return (easinessFactor - 2.5) / -0.012;
}
