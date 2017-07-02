package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Icloud;

public class IcloudSteps {

	Icloud icloud= new Icloud(WebDriverFactory.get());

	@When("I login icloud with '(.*)'")
	public void login(String username){
		icloud.enterEmail(username);
		String password =System.getenv().get("APPLEPASS");
		icloud.enterPassword(password);
		icloud.submit();		
	}
	
	@Then("I should be logged in icloud")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",icloud.checkLogin());
	}
	
	@When("I go to login page")
	public void loginPage(){
		Assert.assertTrue("Expected User to be logged in",icloud.checkAtLoginPage());
	}
	
	@When("I logout icloud")
	public void pressLogout(){
		icloud.logout();
	}
}
