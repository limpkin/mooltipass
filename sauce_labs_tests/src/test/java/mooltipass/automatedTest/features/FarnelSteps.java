package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Farnel;

public class FarnelSteps {
	Farnel farnel= new Farnel(WebDriverFactory.get());

	@When("I login Farnel with '(.*)'")
	public void login(String username){
		farnel.clickContinue();
		farnel.goToLogin();
		farnel.enterEmail(username);
		String password=System.getenv().get("FARNELLPASS");
		farnel.enterPassword(password);
		farnel.submit();
		
	}
	@When("I go to Farnel login page")
	public void pressLogin(){
		
	}
	@Then("I should be logged in Farnel")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",farnel.checkLogin());
	}
	
	@When("I logout Farnel")
	public void pressLogout(){
		farnel.clickDashboard();
		farnel.logout();
	}
}
