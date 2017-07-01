package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.HardwareFR;

public class HardwareFRSteps {
	HardwareFR hFR= new HardwareFR(WebDriverFactory.get());

	@When("I login HardwareFR with '(.*)'")
	public void login(String username){
		hFR.goToLogin();
		hFR.enterEmail(username);
		String password =System.getenv().get("HFRPASS");
		hFR.enterPassword(password);
		hFR.submit();
		
	}
	@When("I go to HardwareFR login page")
	public void pressLogin(){
		hFR.goToLogin();
		Assert.assertTrue("Expected to be at login page", hFR.checkAtLoginPage());
		
	}
	@Then("I should be logged in HardwareFR")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",hFR.checkLogin());
	}
	
	@When("I logout HardwareFR")
	public void pressLogout(){
		hFR.logout();
	}
}
