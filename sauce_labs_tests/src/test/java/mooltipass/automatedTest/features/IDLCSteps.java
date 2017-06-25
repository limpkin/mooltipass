package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.IDLC;

public class IDLCSteps {
	IDLC idlc= new IDLC(WebDriverFactory.get());

	@When("I login IDLC with '(.*)' and '(.*)'")
	public void login(String username,String password){
		idlc.goToLogin();
		idlc.enterEmail(username);
		idlc.enterPassword(password);
		idlc.submit();
		
	}
	@When("I go to IDLC login page")
	public void pressLogin(){
		idlc.goToLogin();
		Assert.assertTrue("Expected to be at login page", idlc.checkAtLoginPage());
		
	}
	@Then("I should be logged in IDLC")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",idlc.checkLogin());
	}
	
	@When("I logout IDLC")
	public void pressLogout(){
		idlc.goTodDashboard();
		idlc.logout();
	}
}
