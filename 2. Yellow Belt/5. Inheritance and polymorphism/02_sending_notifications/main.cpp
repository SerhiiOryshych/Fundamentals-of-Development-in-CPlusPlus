#include <iostream>
#include <utility>

using namespace std;

void SendSms(const string &number, const string &message) {
    cout << "Send '" << message << "' to number " << number << endl;
}

void SendEmail(const string &email, const string &message) {
    cout << "Send '" << message << "' to e-mail " << email << endl;
}

class INotifier {
public:
    virtual void Notify(const string &message) const = 0;
};

class SmsNotifier : public INotifier {
public:
    explicit SmsNotifier(string phone_number) : phone_number_(std::move(phone_number)) {}

    void Notify(const string &message) const override {
        SendSms(phone_number_, message);
    }

private:
    const string phone_number_;
};

class EmailNotifier : public INotifier {
public:
    explicit EmailNotifier(string email) : email_(std::move(email)) {}

    void Notify(const string &message) const override {
        SendEmail(email_, message);
    }

private:
    const string email_;
};
