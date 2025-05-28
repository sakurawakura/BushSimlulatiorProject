#include "Tree.h"
#include <string>    // For std::string
#include <vector>    // For std::vector
#include <limits>    // For std::numeric_limits
#include <sstream>   // For std::istringstream
#include <iostream>  // For std::cerr, std::endl
#include <algorithm> // For std::min

//Maximum area of a branch before it will no longer sprout new branches
const float NEW_BRANCH_THRESHOLD = 5000;


//Required nutrients and water for a new branch to grow
const float NEW_BRANCH_REQUIREMENT = 3;

//Scales the amount that branches grow by with a given amount of food
const float BRANCH_GROWTH_AMOUNT = 50;

//Chance of each existing branch growing a new branch
const float NEW_BRANCH_PROBABILITY = 0.7;

// Resource consumption constants per branch per growth cycle
const float WATER_CONSUMPTION_PER_BRANCH = 0.1f; 
const float NUTRIENT_CONSUMPTION_PER_BRANCH = 0.05f;


Tree::Tree(float initialWater, float initialNutrients, Branch* trunk): waterLevel(initialWater), 
nutrientLevel(initialNutrients), maxIndex(1) {
    //Adds the trunk as the first branch in the list
    branchList.push_back(trunk);

    //sets a seed for randomly generated numbers
    long int t = static_cast<long int> (time(NULL));
    srand(t);

    //Updates the max water and nutrients of the tree
    updateMaxConstraints();

    nextFruitId = 0; // Initialize nextFruitId
}

Tree::~Tree(){
    //Deletes all of the branches
    for(int i = 0; i < branchList.size(); i++){
        delete branchList[i];
    }
}

float Tree::addWater(float litres){
    if (litres < 0) litres = 0; // Do not add negative amounts

    float availableCapacity = maxWater - waterLevel;
    if (availableCapacity < 0) availableCapacity = 0; // Cannot be negative capacity

    float amountActuallyAdded = std::min(litres, availableCapacity);
    
    waterLevel += amountActuallyAdded;
    // Ensure waterLevel does not exceed maxWater due to potential floating point inaccuracies if very close.
    // However, std::min should prevent this if availableCapacity is calculated correctly.
    // For safety, one might clamp: waterLevel = std::min(waterLevel, maxWater);
    // But let's keep it simple as per the request.

    return amountActuallyAdded;
}

float Tree::addNutrients(float kilograms){
    if (kilograms < 0) kilograms = 0; // Do not add negative amounts

    float availableCapacity = maxNutrients - nutrientLevel;
    if (availableCapacity < 0) availableCapacity = 0; // Cannot be negative capacity

    float amountActuallyAdded = std::min(kilograms, availableCapacity);
    
    nutrientLevel += amountActuallyAdded;
    // Similar note about potential clamping for nutrientLevel if needed, but keep simple.

    return amountActuallyAdded;
}

void Tree::removeWater(float litres){
    waterLevel -= litres;
}

void Tree::removeNutrients(float kilograms){
    nutrientLevel -= kilograms;
}

void Tree::addBranches(vector<Branch*> newBranches){
    //Adds the additional branches to the tree
    branchList.insert(branchList.end(), newBranches.begin(), newBranches.end());

    //Updates the max water and nutrients of the tree
    updateMaxConstraints();
}

/**
 * @brief Simulates the growth of the tree for one turn.
 * This includes managing branch sustenance (incrementing neglect counters, checking for death),
 * calculating overall growth based on available resources, applying growth to each branch,
 * potentially spawning new branches, and spawning fruits.
 * @param waterConsumed Output parameter: amount of water consumed by the tree this turn.
 * @param nutrientsConsumed Output parameter: amount of nutrients consumed by the tree this turn.
 * @param widthIncreases Output parameter: list of width increases for each branch.
 * @param lengthIncreases Output parameter: list of length increases for each branch.
 * @param branchesGrown Output parameter: list of indices for newly grown branches.
 */
void Tree::grow(float &waterConsumed, float &nutrientsConsumed, 
    vector<float> &widthIncreases, vector<float> &lengthIncreases, vector<int> &branchesGrown){

    // === Branch Sustenance and Lifecycle Management ===
    // Iterate through all branches to update their sustenance status before growth.
    for (Branch* branch : branchList) {
        if (branch->getIsAlive()) { // Only process living branches.
            // Increment counters for turns without water and nutrients.
            branch->incrementTurnsWithoutWater();
            branch->incrementTurnsWithoutNutrients();

            // Check if the branch should die due to neglect.
            if (branch->getTurnsWithoutWater() > Branch::MAX_TURNS_WITHOUT_SUSTENANCE ||
                branch->getTurnsWithoutNutrients() > Branch::MAX_TURNS_WITHOUT_SUSTENANCE) {
                branch->setIsAlive(false); // Mark the branch as dead.
                // std::cout << "Branch " << branch->getIndex() << " has died due to neglect." << std::endl; // Optional debug message.
            }
        }
    }

    // === Growth Calculation ===
    // Determine overall growth amount based on the minimum of available water and nutrients.
    float growthAmount = min(waterLevel, nutrientLevel);
    float branchGrowthAmount = 0;
    if (!branchList.empty()) { // Avoid division by zero if branchList is empty
        branchGrowthAmount = BRANCH_GROWTH_AMOUNT * growthAmount / branchList.size();
    }

    // Calculate consumption based on the number of living branches
    float currentWaterConsumption = 0.0f;
    float currentNutrientConsumption = 0.0f;
    int livingBranchesCount = 0;

    for (Branch* branch : branchList) {
        if (branch->getIsAlive()) {
            livingBranchesCount++;
        }
    }
    
    // Only consume if there are living branches
    if (livingBranchesCount > 0) { 
        currentWaterConsumption = livingBranchesCount * WATER_CONSUMPTION_PER_BRANCH;
        currentNutrientConsumption = livingBranchesCount * NUTRIENT_CONSUMPTION_PER_BRANCH;
    }

    // Clamp consumption to ensure resource levels don't go below zero due to this consumption step
    currentWaterConsumption = std::min(currentWaterConsumption, waterLevel);
    currentNutrientConsumption = std::min(currentNutrientConsumption, nutrientLevel);

    waterLevel -= currentWaterConsumption;
    nutrientLevel -= currentNutrientConsumption;

    //Updates output variables based on the amount of water and nutrients consumed
    waterConsumed = currentWaterConsumption;
    nutrientsConsumed = currentNutrientConsumption;

    int currentNumBranches = branchList.size();

    const float FRUIT_SPAWN_PROBABILITY = 0.1f;

    for(int branchIndex = 0; branchIndex < currentNumBranches; branchIndex++){

        float widthGrowth;
        float lengthGrowth;

        //Grows the branch by the calculated amount
        branchList[branchIndex]->grow(branchGrowthAmount, widthGrowth, lengthGrowth);


        //Adds the growth amounts to the corresponding lists
        widthIncreases.push_back(widthGrowth);
        lengthIncreases.push_back(lengthGrowth);

        //Moves all of the child branches in accordance with the branch's growth
        //Gets children of current branch
        vector<int> childIndices = branchList[branchIndex]->getChildren();

        //Gets new position of the tip of the current branch
        float newTipX;
        float newTipY;
        branchList[branchIndex]->getTipPos(newTipX, newTipY);

        if (branchList[branchIndex]->getIsAlive() && (float)rand()/RAND_MAX < FRUIT_SPAWN_PROBABILITY) {
            FruitType spawnedFruitType;
            cv::Scalar spawnedFruitColor;
            float randVal = (float)rand() / RAND_MAX;

            if (randVal < 0.05f) { // 5% Gold
                spawnedFruitType = FruitType::GOLD;
                spawnedFruitColor = CV_RGB(255, 215, 0);
            } else if (randVal < 0.30f) { // 25% Blue
                spawnedFruitType = FruitType::BLUE;
                spawnedFruitColor = CV_RGB(0, 0, 255);
            } else { // 70% Red
                spawnedFruitType = FruitType::RED;
                spawnedFruitColor = CV_RGB(255, 0, 0);
            }
            
            float fruitRadius = 5.0f;

            fruitsList.emplace_back(cv::Point2f(newTipX, newTipY), spawnedFruitColor, fruitRadius, spawnedFruitType, nextFruitId++, branchList[branchIndex]->getIndex());
        }

        //Adds a new branch if the tree has the required nutrients and water
        if(min(nutrientLevel, waterLevel) > NEW_BRANCH_REQUIREMENT && 
        branchList[branchIndex]->getSize() < NEW_BRANCH_THRESHOLD &&
        (float)rand()/RAND_MAX < NEW_BRANCH_PROBABILITY){

            //Gets the angle of the current branch
            float currentBranchAngle = branchList[branchIndex]->getAngle();

            //Generates a random number between -70 and 70
            float newAngle = 140*((float)(rand()) /RAND_MAX-0.5);
            Branch* newBranch = new Branch(maxIndex, branchList[branchIndex]->getIndex(), newAngle, 50, 10, newTipX, newTipY);
            branchList.push_back(newBranch);

            //Adds the new branch index to the list of new branches grown
            branchesGrown.push_back(maxIndex);
           
            branchList[branchIndex]->addChild(maxIndex);
            
            //Increments the highest index
            maxIndex++;
        }

    }

    //Updates positions of branches
    updateBranchPos();

    //Updates the max water and nutrients of the tree
    updateMaxConstraints();

}

/**
 * @brief Resets the 'turnsWithoutWater' counter for all living branches in the tree.
 * This is typically called after a watering action.
 */
void Tree::resetAllBranchWaterCounters() {
    for (Branch* branch : branchList) {
        if (branch->getIsAlive()) { // Only affect living branches.
            branch->resetTurnsWithoutWater(); 
        }
    }
}

/**
 * @brief Resets the 'turnsWithoutNutrients' counter for all living branches in the tree.
 * This is typically called after a fertilising action.
 */
void Tree::resetAllBranchNutrientCounters() {
    for (Branch* branch : branchList) {
        if (branch->getIsAlive()) { // Only affect living branches.
            branch->resetTurnsWithoutNutrients();
        }
    }
}

void Tree::pruneBranch(int branchIndex, vector<Branch*> &removedBranches) {
    int currentBranchListIndex = findBranch(branchIndex);

    if (currentBranchListIndex == -1) {
        std::cerr << "Error: Attempted to prune non-existent branch with index: " << branchIndex << "." << std::endl;
        return;
    }

    //Gets children of branch
    vector<int> childIndices = branchList[currentBranchListIndex]->getChildren();

    vector<Branch*> prunedBranchesLocal; // Use a local vector to accumulate

    prunedBranchesLocal.push_back(branchList[currentBranchListIndex]);

    // It's important to remove the branch from its parent *before* its memory is deleted or it's removed from branchList,
    // to correctly update parent's childIndices.
    // The actual removal from branchList and memory deletion should happen after processing children,
    // or be handled carefully if done here (e.g. if removeBranches deletes memory).
    // For now, let's assume removeBranches handles removal from branchList and parent, but not memory deletion.
    // We'll collect all branches to be deleted and then remove them.

    // The original code called removeBranches({branchIndex}) here, which removes it from the list.
    // This can be problematic if the branch pointer is needed later or if child pruning relies on its presence.
    // A safer approach is to mark for deletion or handle deletion at the end.
    // However, to stick to the refactoring instructions and minimal logic change:
    // We will collect all branches to be removed (this one and its descendants)
    // and then call removeBranches once with all of them.
    // This means the current `removeBranches({branchIndex});` will be moved effectively.

    vector<Branch*> childrenPruned;
    for(int childIdx : childIndices) {
        pruneBranch(childIdx, childrenPruned); // Recursive call
        for(Branch* p_branch : childrenPruned) {
            prunedBranchesLocal.push_back(p_branch);
        }
        childrenPruned.clear(); // Clear for next iteration
    }

    // Now, prepare the list of indices to be removed by removeBranches.
    // This part of the logic changes slightly from the original, as removeBranches is called once at the end (implicitly).
    // The output parameter `removedBranches` should contain all branches that were "pruned".
    // The actual deletion from `branchList` and memory is handled by `removeBranches` or the caller.

    // The current structure has `removeBranches` called inside the loop in the original.
    // Let's simplify to collect all branches to be removed.
    // The `removedBranches` parameter will effectively be this collection.

    // The original `removeBranches({branchIndex});` call needs to be addressed.
    // If `removeBranches` deletes the branch from `branchList`, then `branchList[currentBranchListIndex]`
    // would be invalid for subsequent operations in this specific call if not handled.
    // However, the recursive calls handle their own removals.
    // The primary branch being pruned (`branchIndex`) needs to be removed.

    // For this refactoring, assume `removeBranches` just removes from the list and parent.
    // We are passing `removedBranches` by reference.
    
    // The original logic was:
    // 1. Add current branch to prunedBranches.
    // 2. Call removeBranches({branchIndex}) -> this removes it from the main list AND from its parent.
    // 3. Recursively call pruneBranch for children, adding their results to prunedBranches.
    // This seems correct. The recursive call will handle children.
    // The main branch needs to be removed from its parent's list of children
    // and from the tree's main branchList. The `removeBranches` method does this.
    
    // So, the call to removeBranches for the *current* branchIndex should still happen.
    // The `removedBranches` vector is the accumulator for all branches that are part of this pruned subtree.

    // Let's refine the accumulation. `removedBranches` is an output parameter.
    removedBranches.clear(); // Clear it first, as it's an out-param for this specific call scope
    removedBranches.push_back(branchList[currentBranchListIndex]); // Add the current branch

    // Recursively prune children and collect their results
    vector<Branch*> tempPrunedChildren;
    for(int childIdx : childIndices) {
        pruneBranch(childIdx, tempPrunedChildren); // Recursive call
        for(Branch* p_branch : tempPrunedChildren) {
            removedBranches.push_back(p_branch); // Add to the main output
        }
        // tempPrunedChildren is cleared by the recursive call's start or should be if it's purely an out-param.
        // For safety, or if it can accumulate across calls (which it shouldn't as an out-param), clear here.
        tempPrunedChildren.clear(); 
    }
    
    // After collecting this branch and all its descendants to `removedBranches`,
    // this specific branch (`branchIndex`) needs to be removed from the main `branchList`
    // and from its parent's child list. This is done by `removeBranches`.
    // The original code did this *before* the recursive calls.
    // Moving it after might be safer if recursion needed parent context, but pruneBranch starts with findBranch.
    // Let's stick to the original order of removal for the current branch.
    
    // Re-evaluating original logic:
    // `prunedBranches.push_back(branchList[findBranch(branchIndex)]);`
    // `removeBranches({branchIndex});` // This removes the current branch from the tree's list
                                    // and from its parent's child list.
    // Then it iterates `childIndices`. If `removeBranches` modified `childIndices` (it shouldn't directly,
    // but `branchList[currentBranchListIndex]` is now invalid), this would be an issue.
    // `getChildren()` returns a copy, so `childIndices` is stable.

    // Corrected flow based on understanding the original intent and applying refactoring:
    // 1. Find branch. If not found, error and return.
    // 2. Store its children (as copies).
    // 3. Add current branch to `removedBranches` (output parameter).
    // 4. Call `removeBranches` for the current `branchIndex`. This will remove it from `branchList`
    //    and its parent's `childIndices`. *This means `branchList[currentBranchListIndex]` is no longer valid after this point.*
    //    Therefore, step 3 should use a copy or get data before this.
    //    The `prunedBranches.push_back(branchList[currentBranchListIndex]);` in original was fine because `removeBranches`
    //    doesn't delete the Branch* pointer itself, just removes it from lists. The pointer is still valid.

    // Let's rewrite with the refactoring instructions, keeping original logic sequence where possible.
    // The `removedBranches` parameter should accumulate. The problem statement says "The core recursive logic for collecting
    // pruned branches should remain the same." This implies `removedBranches` is an accumulator.
    // The local `prunedBranches` in original was a bit confusing.
    
    // Version 2, aiming to be closer to original recursive accumulation, with optimized lookup:
    // `removedBranches` is an out-parameter, it should contain the list of branches pruned *by this call and its children*.
    // It should be cleared at the start of the function if it's an out-parameter for *this level* of recursion.
    // Or, if it's an accumulator passed down, it shouldn't be cleared.
    // Given typical C++ out-vector patterns, it's usually cleared or replaced.
    // Let's assume it's meant to return branches pruned from this call downwards.
    
    removedBranches.clear(); // Standard practice for vector out-parameters.
    Branch* currentBranchPtr = branchList[currentBranchListIndex];
    removedBranches.push_back(currentBranchPtr);

    // Original code called removeBranches for the current branch *before* recursive calls.
    // This is important because it detaches the current branch from the tree structure.
    removeBranches({branchIndex}); // Removes from branchList and parent's child list.
                                 // After this, currentBranchListIndex is invalid for indexing branchList.
                                 // currentBranchPtr is still valid.

    vector<Branch*> childrenPrunedAccumulator;
    for (int childIdx : childIndices) { // childIndices is a copy, so it's stable.
        pruneBranch(childIdx, childrenPrunedAccumulator); // Recursive call
        for(Branch* p_branch : childrenPrunedAccumulator) {
            removedBranches.push_back(p_branch); // Accumulate results
        }
        // childrenPrunedAccumulator will be cleared by the next recursive call's start.
    }
    // The `removedBranches` now contains `currentBranchPtr` and all its descendant branches.
    // The `removeBranches` call above handled the removal of `currentBranchPtr` from the tree's main list.
    // The recursive calls to `pruneBranch` handled removal of children from the tree's main list.
}

void Tree::removeBranches(vector<int> branchIndices){
    //Loops through given list of branches
    for(int branchIdToRemove : branchIndices){
        int listIdxOfBranchToErase = findBranch(branchIdToRemove);
        if (listIdxOfBranchToErase == -1) {
            std::cerr << "Warning: Branch with ID " << branchIdToRemove << " not found for removal in Tree::removeBranches." << std::endl;
            continue; // Skip to the next ID
        }

        // Get parent details BEFORE erasing the current branch
        int parentId = branchList[listIdxOfBranchToErase]->getParentIndex();
        // Detach from parent
        // Assuming getParentIndex() returns a special value like 0 or -1 if there's no parent (e.g. for the trunk)
        // A branch should not be its own parent. The trunk's parentIndex is -1.
        if (parentId != branchList[listIdxOfBranchToErase]->getIndex() && parentId != -1) { 
            int parentListIdx = findBranch(parentId);
            if (parentListIdx != -1) {
                branchList[parentListIdx]->removeChild(branchIdToRemove);
            } else {
                // This case (parent ID exists but parent branch not found in branchList) might indicate an issue.
                // For example, if the parent was already removed in the same batch.
                // std::cerr << "Warning: Parent branch with ID " << parentId << " not found for child " << branchIdToRemove << std::endl;
            }
        }
        
        // Now erase the branch itself
        branchList.erase(branchList.begin() + listIdxOfBranchToErase);
    }

    //Updates the max water and nutrients of the tree
    updateMaxConstraints();
}

void Tree::modifyBranches(vector<float> widthIncreases, vector<float> lengthIncreases){
    //Checks that the modification is valid
    if(widthIncreases.size() != branchList.size() || lengthIncreases.size() != branchList.size()){
        cout << "Error in Tree.modifyBranches(), size of modifying arrays does not match the number of branches in the tree" << endl;
        return;
    }

    //Loops through each of the branches in the tree
    for(int i = 0; i < branchList.size(); i++){
        //Adjusts branch size
        branchList[i]->modifySize(-widthIncreases[i], -lengthIncreases[i]);
        //Decreases age of branch
        branchList[i]->decrementAge();
    }

    //Adjusts positions of branches in accordance with their new sizes
    updateBranchPos();


    //Updates the max water and nutrients of the tree
    updateMaxConstraints();
}

int Tree::findBranch(int index){
    //Loops through the list of branches
    for(int i = 0; i < branchList.size(); i++){
        if(branchList[i]->getIndex() == index) return i;
    }

    return -1;
}

void Tree::updateMaxConstraints(){
    float totalArea = 0;

    for(int i =0; i < branchList.size(); i++){
        totalArea += branchList[i]->getSize();
    }

    //Updates the maximum water and nutrients that can be stored in the tree
    maxWater = totalArea/50;
    maxNutrients = totalArea/50;
}

void Tree::updateBranchPos(){

    
    //Adjusts positions of branches in accordance with their new sizes
    for(int i = 0; i < branchList.size(); i++){
        //Moves child branches to account for the change in size of their parent

        //Gets children of current branch
        vector<int> childIndices = branchList[i]->getChildren();

        //Gets new position of the tip of the current branch
        float newTipX;
        float newTipY;
        branchList[i]->getTipPos(newTipX, newTipY);

        //Loops through all of the children
        for(int i = 0; i < childIndices.size(); i++){
            //Adjusts position of branches
            branchList[findBranch(childIndices[i])]->setPos(newTipX, newTipY);
        }
    }
}

const std::vector<Fruit>& Tree::getFruitsList() const {
    return fruitsList;
}

bool Tree::collectFruitAtPoint(const cv::Point& clickPoint, FruitType& outCollectedFruitType, int& outCollectedFruitId) {
    for (Fruit& fruit : fruitsList) { // Iterate with non-const reference to modify 'collected'
        if (!fruit.collected) {
            // Simple circle collision detection: (x2-x1)^2 + (y2-y1)^2 < r^2
            float distanceSq = pow(static_cast<float>(clickPoint.x) - fruit.position.x, 2) + 
                             pow(static_cast<float>(clickPoint.y) - fruit.position.y, 2);
            if (distanceSq < (fruit.radius * fruit.radius)) {
                fruit.collected = true; // Mark as collected
                outCollectedFruitType = fruit.type;
                outCollectedFruitId = fruit.id;
                return true; // Fruit collected
            }
        }
    }
    return false; // No fruit collected at this point
}

void Tree::draw(Mat* img){
    for(int i = 0; i < branchList.size(); i++){
        branchList[i]->draw(img);
    }
}

int Tree::getClickedIndex(int mouseX, int mouseY) {
    for(int i = 0; i < branchList.size(); i++){
        if(branchList[i]->containsMouse(mouseX, mouseY)){
            return branchList[i]->getIndex();
        }
    }
    return -1;
}

void Tree::printData(){
    cout << "Tree object" << endl;
    cout << "Water level: " << waterLevel;
    cout << "Fertiliser level: " << nutrientLevel;
    cout << "Max water capacity: " << maxWater;
    cout << "Max nutrient capacity: " << maxNutrients;
    cout << "Max branch index: " << maxIndex;

    cout << "Tree object" << endl;

    //Loops through each of the branches in the tree
    cout << "List of branches in the tree: " << endl;
    for(int i = 0; i < branchList.size(); i++){
        branchList[i]->printData();
    }
        
    cout << "Water level: " << waterLevel << endl;;
    cout << "Max water: " << maxWater << endl;
    cout << "Nutrient level: " << nutrientLevel << endl;
    cout << "Max nutrients: " << maxNutrients << endl;

}

void Tree::saveToStream(std::ostream& out) const {
    out << "tree_max_index " << maxIndex << std::endl;
    out << "tree_water_level " << waterLevel << std::endl;
    out << "tree_nutrient_level " << nutrientLevel << std::endl;
    out << "num_branches " << branchList.size() << std::endl;
    for (const Branch* branch : branchList) {
        if (branch) {
            branch->saveToStream(out); // Each branch writes its own newline
        }
    }
    // Save fruit data
    out << "next_fruit_id " << nextFruitId << std::endl;
    out << "num_fruits " << fruitsList.size() << std::endl;
    for (const Fruit& fruit : fruitsList) {
        out << "fruit"
            << " id " << fruit.id
            << " parent_branch_idx " << fruit.parentBranchIndex
            << " type " << static_cast<int>(fruit.type)
            << " pos_x " << fruit.position.x
            << " pos_y " << fruit.position.y
            << " radius " << fruit.radius
            << " color_b " << fruit.color[0] // Assuming BGR order for cv::Scalar
            << " color_g " << fruit.color[1]
            << " color_r " << fruit.color[2]
            << " collected " << fruit.collected // Saving the collected status
            << std::endl;
    }
}

Tree* Tree::loadFromStream(std::istream& in, int windowWidth, int windowHeight) {
    std::string k_max_idx, k_water, k_nutrient, k_num_branches;
    int p_maxIndex = 0;
    float p_waterLevel = 0.0f;
    float p_nutrientLevel = 0.0f;
    int p_num_branches = 0;

    // Read tree properties
    in >> k_max_idx >> p_maxIndex
       >> k_water >> p_waterLevel
       >> k_nutrient >> p_nutrientLevel
       >> k_num_branches >> p_num_branches;

    if (in.fail() || k_max_idx != "tree_max_index" || k_water != "tree_water_level" ||
        k_nutrient != "tree_nutrient_level" || k_num_branches != "num_branches") {
        std::cerr << "Error: Failed to load Tree properties. Cannot load tree." << std::endl;
        return nullptr;
    }

    // Consume the rest of the line after num_branches
    std::string dummy_line;
    std::getline(in, dummy_line);

    std::vector<Branch*> loadedBranches;
    loadedBranches.reserve(p_num_branches);

    for (int i = 0; i < p_num_branches; ++i) {
        Branch loadedBranch = Branch::loadFromStream(in); // Assumes Branch::loadFromStream reads one line
        if (loadedBranch.getIndex() == -1) { // Branch::loadFromStream returns default branch with index -1 on failure
            std::cerr << "Error loading branch " << i << " from stream." << std::endl;
            for (Branch* b : loadedBranches) delete b; // Cleanup already loaded branches
            return nullptr;
        }
        loadedBranches.push_back(new Branch(loadedBranch));
    }

    if (loadedBranches.empty() && p_num_branches > 0) {
         std::cerr << "Error: No branches loaded despite num_branches (" << p_num_branches << ") > 0." << std::endl;
         return nullptr;
    }

    Tree* loadedTree = nullptr;
    if (!loadedBranches.empty()) {
        Branch* trunk = new Branch(*loadedBranches[0]);
        loadedTree = new Tree(p_waterLevel, p_nutrientLevel, trunk);
        delete loadedTree->branchList[0];
        loadedTree->branchList.clear();
        for (Branch* b_ptr : loadedBranches) {
            loadedTree->branchList.push_back(b_ptr);
        }
    } else {
         std::cout << "No branches in save file or failed to load branches. Creating default tree." << std::endl;
         Branch* defaultTrunk = new Branch(0, -1, 0.0f, 50.0f, 10.0f,  windowWidth/ 2.0f, (float)windowHeight);
         loadedTree = new Tree(p_waterLevel, p_nutrientLevel, defaultTrunk);
    }

    loadedTree->maxIndex = p_maxIndex;

    // Fruit Data Reading
    std::string k_next_fruit_id, k_num_fruits;
    int temp_next_fruit_id = 0; // Use temporary for reading
    int num_fruits = 0;

    in >> k_next_fruit_id >> temp_next_fruit_id >> k_num_fruits >> num_fruits;

    if (in.fail() || k_next_fruit_id != "next_fruit_id" || k_num_fruits != "num_fruits") {
        std::cerr << "Error: Failed to load fruit metadata. Fruits will not be loaded." << std::endl;
        // Proceed without loading fruits, nextFruitId remains its default, fruitsList is empty.
    } else {
        loadedTree->nextFruitId = temp_next_fruit_id; // Assign if read successfully
        loadedTree->fruitsList.clear();
        loadedTree->fruitsList.reserve(num_fruits);

        for (int i = 0; i < num_fruits; ++i) {
            Fruit tempFruit;
            int fruit_type_int;
            bool fruit_collected_bool;
            std::string k_fruit, k_id, k_parent_idx, k_type, k_pos_x, k_pos_y, k_radius, k_col_b, k_col_g, k_col_r, k_collected;

            in >> k_fruit
               >> k_id >> tempFruit.id
               >> k_parent_idx >> tempFruit.parentBranchIndex
               >> k_type >> fruit_type_int
               >> k_pos_x >> tempFruit.position.x
               >> k_pos_y >> tempFruit.position.y
               >> k_radius >> tempFruit.radius
               >> k_col_b >> tempFruit.color[0]
               >> k_col_g >> tempFruit.color[1]
               >> k_col_r >> tempFruit.color[2]
               >> k_collected >> fruit_collected_bool;
            
            if (in.fail() || k_fruit != "fruit" || k_id != "id" || k_parent_idx != "parent_branch_idx" ||
                k_type != "type" || k_pos_x != "pos_x" || k_pos_y != "pos_y" || k_radius != "radius" ||
                k_col_b != "color_b" || k_col_g != "color_g" || k_col_r != "color_r" || k_collected != "collected") {
                std::cerr << "Error: Failed to load one fruit. Skipping it." << std::endl;
                // Consume the rest of the potentially malformed line for this fruit
                std::string bad_fruit_line;
                std::getline(in, bad_fruit_line);
                in.clear(); // Clear error flags to allow further reading for next fruits
                continue; 
            }
            tempFruit.type = static_cast<FruitType>(fruit_type_int);
            tempFruit.collected = fruit_collected_bool;
            loadedTree->fruitsList.push_back(tempFruit);
        }
    }
    
    loadedTree->updateMaxConstraints();
    loadedTree->updateBranchPos();

    return loadedTree;
}