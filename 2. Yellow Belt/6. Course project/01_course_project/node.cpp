#include "node.h"

EmptyNode::EmptyNode() = default;

bool EmptyNode::Evaluate(const Date &date, const string &event) const {
    return true;
}

DateComparisonNode::DateComparisonNode(const Comparison &cmp, const Date &date) : cmp_(cmp), date_(date) {}

bool DateComparisonNode::Evaluate(const Date &date, const string &event) const {
    switch (cmp_) {
        case Comparison::Less:
            return date < date_;
        case Comparison::LessOrEqual:
            return (date < date_ || date == date_);
        case Comparison::Greater:
            return date > date_;
        case Comparison::GreaterOrEqual:
            return (date > date_ || date == date_);
        case Comparison::Equal:
            return date == date_;
        case Comparison::NotEqual:
            return !(date == date_);
    }
}

EventComparisonNode::EventComparisonNode(const Comparison &cmp, string event) : cmp_(cmp), event_(std::move(event)) {}

bool EventComparisonNode::Evaluate(const Date &date, const string &event) const {
    switch (cmp_) {
        case Comparison::Less:
            return event < event_;
        case Comparison::LessOrEqual:
            return (event < event_ || event == event_);
        case Comparison::Greater:
            return event > event_;
        case Comparison::GreaterOrEqual:
            return (event > event_ || event == event_);
        case Comparison::Equal:
            return event == event_;
        case Comparison::NotEqual:
            return !(event == event_);
    }
}

LogicalOperationNode::LogicalOperationNode(const LogicalOperation &logical_operation, const shared_ptr<Node> &left,
                                           const shared_ptr<Node> &right) : logical_operation_(logical_operation),
                                                                            left_(left), right_(right) {}

bool LogicalOperationNode::Evaluate(const Date &date, const string &event) const {
    switch (logical_operation_) {
        case LogicalOperation::Or:
            return left_->Evaluate(date, event) || right_->Evaluate(date, event);
        case LogicalOperation::And:
            return left_->Evaluate(date, event) && right_->Evaluate(date, event);
    }
}
