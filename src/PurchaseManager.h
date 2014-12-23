#pragma once

#include <string>
#include <map>

//ut_core
#include "string_type.h"
#include "http.h"

//orderLib
#include <orderLib/Key.h>
#include <orderLib/Reconciliation.h>

//######################################################################
class PurchaseManager {
public:
    //For recording a newly purchased key.  Throws and does nothing if the key is not valid.
    static void saveAndActivateKey(const std::string &licenseKey);

    //Returns a json string representing the current known customer information.
    //Throws on error
    static tstring getCustomerJson();

    //Returns the customer ID
    static tstring getCustomerId() throw();

    //Records the specified customer id as the current known customer.  Use this sparingly!
    //Throws on error
    static void saveCustomerId(const tstring& customerId);

    //Checks saved keys, downloads current customer keys, and sets state for all purchases.
    static void reconcilePurchaseState() throw();

    //Returns the url for managing the customer connection to the client.
    static std::string getUrl_customerManagement();

private:
    //No instantiating.
    PurchaseManager() {}
    PurchaseManager(const PurchaseManager &) {}
    ~PurchaseManager() {}

    //For getting info in order to make the computerId.
    typedef struct {
        uint32 cpu_id[3];
        uint32 cpu_model[4];
        uint32 drive_serial;
    } computerIdInfo_t;
    
    //Returns a map of svcCode -> keyString for all current purchases.
    static orderLib::Reconciliation::t_purchaseMap getAllSavedPurchases();

    //Returns a map of all known services, with empty keys.  This helps us manage un-deployment of expired services.
    static orderLib::Reconciliation::t_purchaseMap getEmptyServiceMap();

    //Internal function for saving the state of a purchase.
    static void setServiceState(const std::string &serviceCode, orderLib::Key &keyObj, orderLib::Key &oldKeyObj);
    static void setServiceState_utAdFree(orderLib::Key &keyObj, orderLib::Key &oldKeyObj);

    static void reconcilePurchaseState_continue(const orderLib::Reconciliation::t_purchaseMap *serverPurchases) throw();
    static void reconcilePurchaseState_handleFailure() throw();
    static void reconcilePurchaseState_callback(orderLib::Reconciliation::t_customerInfoMap *cust, orderLib::Reconciliation::t_purchaseMap *purchases) throw();

};
   
/*
  Local Variables:
  c-basic-offset: 4
  End:
*/
