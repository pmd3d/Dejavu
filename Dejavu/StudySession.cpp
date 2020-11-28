#include "Dejavu.h"

using namespace jlimdev;

// -- pattern match next review item variant...
struct ReviewItemAfterCorrect
{
    Timestamp now;
    DifficultyRating difficultyRating;

    ReviewItem operator()(const NeverReviewed& n)
    {
        return std::move(PreviouslyFirstCorrect{ difficultyRating, now });
    }
    ReviewItem operator()(const PreviouslyIncorrect& p)
    {
        return std::move(PreviouslyFirstCorrect{ difficultyRating, now });
    }
    ReviewItem operator()(const PreviouslyFirstCorrect& p)
    {
        return std::move(PreviouslyCorrect{ difficultyRating, now, p.reviewDate });
    }
    ReviewItem operator()(const PreviouslyCorrect& p)
    {
        return std::move(PreviouslyCorrect{ difficultyRating, now, p.reviewDate });
    }
};

StudySession::StudySession(const IReviewStrategy& reviewStrategy, uint maxNewCard, uint maxExistingCard, uint cardLimit) noexcept
    : _reviewStrategy(reviewStrategy), _newCardsReturned(0), _existingCardsReturned(0),
      _newCardMax(maxNewCard), _existingCardMax(maxExistingCard), _currentIndex(0), 
      _cardLimit(cardLimit)
{
}

void StudySession::Reset()
{
    _cards.clear();
    _visit.clear();
    _newCardsReturned = 0;
    _existingCardsReturned = 0;
    _currentIndex = 0;
}

void StudySession::AddNeverReviewed()
{
    // -- this seems kind of a problem. Not sure how to start it off if it has no history.
    // -- in the original code, Never Reviewed test case started with easiest...
    // -- but seems like would want more reviews initially for new stuff...
    // -- so changing it to most difficult...
    AddItem(NeverReviewed{ DifficultyRatingMostDifficult });
}

void StudySession::AddPreviouslyIncorrect(uint difficultyRating, Timestamp reviewDate)
{
    AddItem(PreviouslyIncorrect{ difficultyRating, reviewDate });
}

void StudySession::AddPreviouslyFirstCorrect(uint difficultyRating, Timestamp reviewDate)
{
    AddItem(PreviouslyFirstCorrect{ difficultyRating, reviewDate });
}

void StudySession::AddPreviouslyCorrect(uint difficultyRating, Timestamp reviewDate, Timestamp previousCorrectReview)
{
    AddItem(PreviouslyCorrect{ difficultyRating, reviewDate, previousCorrectReview });
}

uint StudySession::UpdateCard(uint i, const ReviewOutcome& outcome, uint now)
{
    _cards.at(i) = MapItem(i, outcome, now);

    return GetNextReviewTime(i, now);
}

uint StudySession::GetNextReviewTime(uint i, uint now) const
{
    return _reviewStrategy.NextReview(_cards.at(i), now);
}

const ReviewItem& StudySession::At(uint i) const
{
    return _cards.at(i);
}

// returns optional of index of next card or null option if done
std::optional<uint> StudySession::NextReview(Timestamp now)
{
    // -- finda card to review

    for (uint iCount = 0; iCount < _cards.size(); iCount++)
    {
        // cycle through the entire vector starting from current index...
        const uint i = (_currentIndex + iCount) % _cards.size();
        const uint nextI = (i + 1) % _cards.size();

        if (_visit.at(i) == ReviewState::Wrong)
        {
            _currentIndex = nextI;
            // wrong will always be marked for review until it is right...
            return std::optional<uint>(i);
        }
        else if (_visit.at(i) == ReviewState::Unvisited)
        {
            // unvisited cards usually get reviewed, but put a cap on them since session time has limited attention span...
            if (IsDue(_cards.at(i), now) && IsNewItem(_cards.at(i)) && _newCardsReturned + 1 <= _newCardMax)
            {
                _newCardsReturned++;
                _currentIndex = nextI;
                return std::optional<uint>(i);
            }
            else if (IsDue(_cards.at(i), now) && !IsNewItem(_cards.at(i)) && _existingCardsReturned + 1 <= _existingCardMax)
            {
                _existingCardsReturned++;
                _currentIndex = nextI;
                return std::optional<uint>(i);
            }
        }
    }

    return std::nullopt;
}

void StudySession::AddItem(const ReviewItem& item)
{
    if (_cards.size() > _cardLimit)
    {
        exit(1);
    }

    _cards.emplace_back(item);
    _visit.emplace_back(ReviewState::Unvisited);
}

// -- make next state transition by using user response and pattern matching on current card...
ReviewItem StudySession::MapItem(uint i, const ReviewOutcome& outcome, Timestamp now)
{
    const ReviewItem& item = _cards.at(i);
    const DifficultyRating difficultyRating = _reviewStrategy.AdjustDifficulty(item, outcome);

    if (outcome == ReviewOutcome::Incorrect)
    {
        _visit.at(i) = ReviewState::Wrong;
        
        return std::move(PreviouslyIncorrect{ difficultyRating, now });
    }
    else
    {
        _visit.at(i) = ReviewState::Visited;

        return std::visit(ReviewItemAfterCorrect{ now, difficultyRating }, item);
    }
}

bool StudySession::IsDue(const ReviewItem& item, const Timestamp& now) const
{
    const auto nextReview = _reviewStrategy.NextReview(item, now);
    return nextReview <= now;
}

bool StudySession::IsNewItem(const ReviewItem& item) const noexcept
{
    return item.index() == 0;
}
