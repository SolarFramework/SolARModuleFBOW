/**
 * @copyright Copyright (c) 2017 B-com http://www.b-com.com/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SolARFBOWHelper.h"

namespace SolAR {
namespace MODULES {
namespace FBOW {

datastructure::BoWFeature SolARFBOWHelper::fbow2Solar(const fbow::fBow& fbow)
{
    datastructure::BoWFeature bowFeature;
    for (const auto& it : fbow)
        bowFeature[it.first] = it.second.var;
    return bowFeature;
}

datastructure::BoWLevelFeature SolARFBOWHelper::fbow2Solar(const fbow::fBow2& fbow2)
{
    return fbow2;
}

double SolARFBOWHelper::distanceBoW(const datastructure::BoWFeature& bow1, const datastructure::BoWFeature& bow2, ScoringType scoreType)
{
    datastructure::BoWFeature::const_iterator bow1_it = bow1.begin();
    datastructure::BoWFeature::const_iterator bow2_it = bow2.begin();
    datastructure::BoWFeature::const_iterator bow1_end = bow1.end();
    datastructure::BoWFeature::const_iterator bow2_end = bow2.end();

    double score = 0.;
    while(bow1_it != bow1_end && bow2_it != bow2_end)
    {
        const auto& w1 = bow1_it->second;
        const auto& w2 = bow2_it->second;
        if(bow1_it->first == bow2_it->first)
        {
            switch(scoreType) { 
            case ScoringType::L2_NORM:
                score += w1 * w2;
                break;
            case ScoringType::L1_NORM:
                score += fabs(w1 - w2) - fabs(w1) - fabs(w2);
                break;
            case ScoringType::CHI_SQUARE:
                if(w1 + w2 != 0.0)
                    score += w1 * w2 / (w1 + w2);
                break;
            case ScoringType::BHATTACHARYYA:
                score += sqrt(w1 * w2);
                break;
            case ScoringType::DOT_PRODUCT:
                score += w1 * w2;
                break;
            default:
                // unknown scoring type
                break;
            } 
            // move v1 and v2 forward
            ++bow1_it;
            ++bow2_it;
        }
        else if(bow1_it->first < bow2_it->first)
        {
            // move v1 forward
            while(bow1_it!=bow1_end && bow1_it->first < bow2_it->first)
                ++bow1_it;
        }
        else
        {
            // move v2 forward
            while(bow2_it!=bow2_end && bow2_it->first < bow1_it->first)
                ++bow2_it;
        }
    }

    switch(scoreType) {
    case ScoringType::L2_NORM:
        // ||v - w||_{L2} = sqrt( 2 - 2 * Sum(v_i * w_i) )
        //		for all i | v_i != 0 and w_i != 0 )
        // (Nister, 2006)
        if(score >= 1) // rounding errors
            score = 1.0;
        else
            score = 1.0 - sqrt(1.0 - score); // [0..1]
        break;
    case ScoringType::L1_NORM:
        score = -score/2.0;
        break;
    case ScoringType::CHI_SQUARE:
        score = 2. * score; // [0..1]
        break;
    case ScoringType::BHATTACHARYYA:
        break;
    case ScoringType::DOT_PRODUCT:
        break;
    default:
        break;
    }
   
    return score;
}

}
}
}
