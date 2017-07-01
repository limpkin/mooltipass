package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.EBay;

public class EBaySteps {

   EBay ebay= new EBay(WebDriverFactory.get());

	@When("I login EBay with '(.*)'")
	public void login(String username){
		ebay.goToLogin();
		ebay.enterEmail(username);
		String password =System.getenv().get("EBAYPASS");
		ebay.enterPassword(password);
		ebay.submit();
		
	}
	@When("I go to EBay login page")
	public void pressLogin(){
		ebay.goToLogin();
		Assert.assertTrue("Expected to be at login page", ebay.checkAtLoginPage());
		
	}
	@Then("I should be logged in EBay")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",ebay.checkLogin());
	}
	
	@When("I logout EBay")
	public void pressLogout(){
		ebay.goTodDashboard();
		ebay.logout();
	}
}
